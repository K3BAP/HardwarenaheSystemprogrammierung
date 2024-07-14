#include <iostream>
#include <ostream>
#include <vector>
#include <cmath>
#include <sndfile.h>
#include <CL/cl2.hpp>

// Function to read WAV file
std::pair<int, std::vector<float>> read_wav_file(const std::string& filename) {
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Error reading WAV file: " << filename << std::endl;
        exit(1);
    }

    std::vector<float> data(sfinfo.frames);
    sf_read_float(infile, data.data(), sfinfo.frames);
    sf_close(infile);
    std::cout << "Frames: " << sfinfo.frames << std::endl;
    return {sfinfo.samplerate, data};
}

// OpenCL FFT Kernel
const char* kernelSource = R"(
__kernel void fft(__global const float* data, __global float* fft_blocks, int block_size, int offset, int num_blocks) {
    int i = get_global_id(0);
    if (i < num_blocks) {
        for (int j = 0; j < block_size / 2; j++) {
            float real = 0.0;
            float imag = 0.0;
            for (int k = 0; k < block_size; k++) {
                float angle = -2.0f * 3.141592653589793f * j * k / block_size;
                real += data[i * offset + k] * cos(angle);
                imag += data[i * offset + k] * sin(angle);
            }
            fft_blocks[i * (block_size / 2) + j] = sqrt(real * real + imag * imag);
        }
    }
}
)";

// Function to perform FFT on GPU
std::pair<std::vector<float>, std::vector<float>> perform_fft_gpu(cl::Context& context, cl::CommandQueue& queue, const std::vector<float>& data, int block_size, int offset, int sample_rate) {
    cl::Program program(context, kernelSource);
    program.build();

    int num_blocks = (data.size() - block_size) / offset + 1;
    std::cout << "Block count: " << num_blocks << std::endl;
    std::cout << "Data size: " << data.size() << std::endl;

    std::vector<float> frequencies(block_size / 2);
    for (int i = 0; i < block_size / 2; ++i) {
        frequencies[i] = i * static_cast<float>(sample_rate) / block_size;
    }
    std::vector<float> fft_blocks(num_blocks * block_size / 2);

    cl::Buffer data_buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * data.size(), const_cast<float*>(data.data()));
    cl::Buffer fft_buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * fft_blocks.size());

    cl::Kernel kernel(program, "fft");
    kernel.setArg(0, data_buffer);
    kernel.setArg(1, fft_buffer);
    kernel.setArg(2, block_size);
    kernel.setArg(3, offset);
    kernel.setArg(4, num_blocks);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(num_blocks), cl::NullRange);
    queue.finish();
    queue.enqueueReadBuffer(fft_buffer, CL_TRUE, 0, sizeof(float) * fft_blocks.size(), fft_blocks.data());

    return {fft_blocks, frequencies};
}

// Function to print mean amplitudes
void print_mean_amplitudes(const std::vector<float>& fft_blocks, const std::vector<float>& frequencies, float threshold) {
    int block_size_half = fft_blocks.size() / frequencies.size();
    std::vector<float> avg_magnitudes(frequencies.size(), 0.0f);

    for (size_t i = 0; i < frequencies.size(); ++i) {
        float sum = 0.0f;
        for (int j = 0; j < block_size_half; ++j) {
            sum += fft_blocks[i + j * frequencies.size()];
        }
        avg_magnitudes[i] = sum / block_size_half;
    }

    std::cout << "Frequencies with average amplitude above the threshold:\n";
    for (size_t i = 0; i < frequencies.size(); ++i) {
        if (avg_magnitudes[i] > threshold) {
            std::cout << "Frequency: " << frequencies[i] << " Hz, Amplitude: " << avg_magnitudes[i] << "\n";
        }
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <file_path> <block_size> <step_width> <amplitude_threshold>" << std::endl;
        return 1;
    }

    std::string file_path = argv[1];
    int block_size = std::stoi(argv[2]);
    int offset = std::stoi(argv[3]);
    float threshold = std::stof(argv[4]);

    if (!(64 <= block_size && block_size <= 512)) {
        std::cerr << "Block size must be between 64 and 512" << std::endl;
        return 1;
    }
    if (!(1 <= offset && offset <= block_size)) {
        std::cerr << "Offset must be between 1 and block_size" << std::endl;
        return 1;
    }

    auto [sample_rate, data] = read_wav_file(file_path);

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };

    cl::Context context(CL_DEVICE_TYPE_GPU, properties);
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    cl::CommandQueue queue(context, devices[0]);

    auto start_time = std::chrono::high_resolution_clock::now();
    auto [fft_blocks, frequencies] = perform_fft_gpu(context, queue, data, block_size, offset, sample_rate);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    print_mean_amplitudes(fft_blocks, frequencies, threshold);

    std::cout << "Processing time: " << duration << " ms" << std::endl;

    return 0;
}

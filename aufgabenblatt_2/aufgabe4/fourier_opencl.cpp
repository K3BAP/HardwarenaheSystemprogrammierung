#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.h>
#include <complex>
#include <chrono>
#include <CL/opencl.hpp>
#include <clFFT.h>

void fft_clfft(const std::vector<std::complex<double>>& input, std::vector<std::complex<double>>& output, unsigned int block_size, unsigned int step_width, unsigned int num_blocks, unsigned int total_samples) {
    std::vector<cl_float2> input_cl(total_samples);
    std::vector<cl_float2> output_cl(num_blocks * block_size);

    for (unsigned int i = 0; i < total_samples; ++i) {
        input_cl[i].s[0] = input[i].real();
        input_cl[i].s[1] = input[i].imag();
    }

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context context(device);
    cl::CommandQueue queue(context, device);

    cl::Buffer buffer_input(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float2) * total_samples, input_cl.data());
    cl::Buffer buffer_output(context, CL_MEM_WRITE_ONLY, sizeof(cl_float2) * num_blocks * block_size);

    // Setup clFFT
    clfftPlanHandle plan;
    size_t cl_lengths[1] = { block_size };
    clfftSetupData fftSetup;
    clfftInitSetupData(&fftSetup);
    clfftSetup(&fftSetup);
    clfftCreateDefaultPlan(&plan, context(), CLFFT_1D, cl_lengths);

    clfftSetPlanPrecision(plan, CLFFT_SINGLE);
    clfftSetLayout(plan, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    clfftSetResultLocation(plan, CLFFT_OUTOFPLACE);

    clfftBakePlan(plan, 1, &(queue()) , NULL, NULL);

    for (unsigned int i = 0; i < num_blocks; ++i) {
        size_t input_offset = i * step_width;
        if (input_offset + block_size > total_samples) break;

        // Execute the FFT
        clfftEnqueueTransform(plan, CLFFT_FORWARD, 1, &(queue()), 0, NULL, NULL, &buffer_input(), &buffer_output(), NULL);
        queue.finish();
    }

    // Read back the results
    queue.enqueueReadBuffer(buffer_output, CL_TRUE, 0, sizeof(cl_float2) * num_blocks * block_size, output_cl.data());

    for (unsigned int i = 0; i < num_blocks * block_size; ++i) {
        output[i] = std::complex<double>(output_cl[i].s[0], output_cl[i].s[1]);
    }

    // Cleanup clFFT
    clfftDestroyPlan(&plan);
    clfftTeardown();
}

void analyze(const std::string& filepath, int block_size, int step_width, double amplitude_threshold) {
    // Open the WAV file
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return;
    }

    // Ensure block_size and step_width are valid
    if (block_size < 64 || block_size > 512) {
        std::cerr << "Block size must be between 64 and 512" << std::endl;
        sf_close(infile);
        return;
    }
    if (step_width < 1 || step_width > block_size) {
        std::cerr << "Step width must be between 1 and block size" << std::endl;
        sf_close(infile);
        return;
    }

    // Buffer to hold samples
    std::vector<double> samples(sfinfo.frames);
    std::vector<double> amplitude_sums(block_size / 2 + 1, 0.0);
    int block_count = 0;

    // Read the entire file into a buffer for efficient processing
    sf_count_t frames_read = sf_read_double(infile, samples.data(), sfinfo.frames);
    if (frames_read != sfinfo.frames) {
        std::cerr << "Error reading samples from file." << std::endl;
        sf_close(infile);
        return;
    }

    // Calculate the number of blocks to process
    int num_blocks = (sfinfo.frames - block_size + step_width) / step_width;

    // Prepare the input for OpenCL
    std::vector<std::complex<double>> input(sfinfo.frames);
    for (size_t i = 0; i < samples.size(); ++i) {
        input[i] = std::complex<double>(samples[i], 0.0);
    }
    std::vector<std::complex<double>> output(num_blocks * block_size);

    // Perform FFT using clFFT
    fft_clfft(input, output, block_size, step_width, num_blocks, sfinfo.frames);

    // Aggregate the results
    for (int block = 0; block < num_blocks; ++block) {
        for (int i = 0; i <= block_size / 2; ++i) {
            double amplitude = std::abs(output[block * block_size + i]) / block_size;
            amplitude_sums[i] += amplitude;
        }
        block_count++;
    }

    // Average amplitudes
    for (auto& amplitude : amplitude_sums) {
        amplitude /= block_count;
    }

    // Output frequencies with amplitude above threshold
    double frequency_step = static_cast<double>(sfinfo.samplerate) / block_size;
    for (size_t i = 0; i <= block_size / 2; ++i) {
        if (amplitude_sums[i] > amplitude_threshold) {
            std::cout << "Frequency: " << i * frequency_step << " Hz, Amplitude: " << amplitude_sums[i] << std::endl;
        }
    }

    // Cleanup
    sf_close(infile);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <file_path> <block_size> <step_width> <amplitude_threshold>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    int block_size = std::stoi(argv[2]);
    int step_width = std::stoi(argv[3]);
    double amplitude_threshold = std::stod(argv[4]);

    auto start_time = std::chrono::high_resolution_clock::now();
    analyze(filepath, block_size, step_width, amplitude_threshold);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Processing time: " << duration << " ms" << std::endl;

    return 0;
}
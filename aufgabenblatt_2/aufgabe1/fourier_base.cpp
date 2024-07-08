#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.h>
#include <complex>
#include <chrono>

// Function to perform the Cooley-Tukey FFT
void fft(std::vector<std::complex<double>>& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    std::vector<std::complex<double>> even(N / 2);
    std::vector<std::complex<double>> odd(N / 2);

    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
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
    std::vector<double> samples(block_size);
    std::vector<double> amplitude_sums(block_size / 2 + 1, 0.0);
    int block_count = 0;

    // Read the entire file into a buffer for efficient processing
    std::vector<double> file_buffer(sfinfo.frames);
    sf_count_t frames_read = sf_read_double(infile, file_buffer.data(), sfinfo.frames);
    if (frames_read != sfinfo.frames) {
        std::cerr << "Error reading samples from file." << std::endl;
        sf_close(infile);
        return;
    }

    // Process the file in blocks with specified step width
    for (sf_count_t start = 0; start + block_size <= sfinfo.frames; start += step_width) {
        std::copy(file_buffer.begin() + start, file_buffer.begin() + start + block_size, samples.begin());

        // Convert samples to complex numbers for FFT
        std::vector<std::complex<double>> complex_samples(samples.begin(), samples.end());

        // Perform FFT
        fft(complex_samples);

        // Calculate amplitude
        for (size_t i = 0; i <= block_size / 2; ++i) {
            double amplitude = std::abs(complex_samples[i]) / block_size;
            amplitude_sums[i] += amplitude;
        }
        block_count++;
    }

    // Average amplitudes
    for (auto& amplitude : amplitude_sums) {
        amplitude /= block_count;
    }

    // Output frequencies with amplitude above threshold
    double frequency_step = (double)sfinfo.samplerate / block_size;
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
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}

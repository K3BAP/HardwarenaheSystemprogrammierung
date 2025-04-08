#include <sycl/sycl.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

using namespace sycl;

constexpr size_t M = 10000;
constexpr size_t N = 10000;
constexpr size_t K = 10000; // shared dimension for multiplication

// Helper function to initialize a vector with random float values between 0 and 1.
void init_random(std::vector<float> &matrix) {
    std::mt19937 gen(42); // fixed seed for reproducibility
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (auto &elem : matrix) {
        elem = dis(gen);
    }
}

int main() {
    // Allocate and initialize matrices A and B. Matrix C will hold the result.
    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N, 0);

    std::cout << "Initializing matrices..." << std::endl;
    init_random(A);
    init_random(B);

    // Create a SYCL queue with profiling enabled.
    queue q{default_selector{}, property::queue::enable_profiling()};

    std::cout << "Launching kernel on device: "
              << q.get_device().get_info<info::device::name>() << "\n";

    // Open a log file to record each kernel execution time in nanoseconds.
    std::ofstream logfile("runtime_log_matmul.txt", std::ios::out);
    if (!logfile.is_open()) {
        std::cerr << "Error: Could not open log file for writing." << std::endl;
        return 1;
    }

    // Create buffers outside the loop so that you don't reallocate them 100 times.
    {
        buffer<float, 1> bufA(A.data(), range<1>(A.size()));
        buffer<float, 1> bufB(B.data(), range<1>(B.size()));
        buffer<float, 1> bufC(C.data(), range<1>(C.size()));

        // Repeat kernel execution 100 times
        for (int i = 0; i < 100; i++) {
            auto event = q.submit([&](handler &h) {
                // Create accessors for the buffers.
                auto a = bufA.get_access<access::mode::read>(h);
                auto b = bufB.get_access<access::mode::read>(h);
                auto c = bufC.get_access<access::mode::write>(h);

                // Execute the kernel using a 2D ND-range.
                h.parallel_for<class matrix_mult>(
                    nd_range<2>({M, N}, {16, 16}),
                    [=](nd_item<2> item) {
                        size_t row = item.get_global_id(0);
                        size_t col = item.get_global_id(1);
                        float sum = 0.0f;
                        for (size_t k = 0; k < K; ++k) {
                            sum += a[row * K + k] * b[k * N + col];
                        }
                        c[row * N + col] = sum;
                    });
            });
            
            // Wait for the kernel to finish execution.
            event.wait();

            // Get the profiling start and end times.
            auto start = event.get_profiling_info<info::event_profiling::command_start>();
            auto end   = event.get_profiling_info<info::event_profiling::command_end>();
            
            // Calculate elapsed time in nanoseconds.
            auto kernel_time_ns = end - start;

            // Write the execution time for this iteration into the log file.
            logfile << "" << kernel_time_ns << "\n";

            // Optionally print progress information.
            std::cout << "Completed iteration " << i << " with kernel runtime (ns): " << kernel_time_ns << "\n";
        }
    } // End of buffer scope; ensures data synchronization if needed.

    logfile.close();
    std::cout << "All calculations complete.\n";
    return 0;
}

#include <CL/sycl.hpp>
#include <fstream>
#include <iostream>
#include <vector>

using namespace sycl;

int main() {
    // Define the number of elements.
    constexpr size_t num_elements = 500UL * 1024UL * 1024UL; // 500 * 1024 * 1024 floats

    // Allocate and initialize source and destination arrays.
    // For this copy operation, initializing src with constant values is sufficient.
    std::vector<float> src(num_elements, 1.0f);
    std::vector<float> dst(num_elements, 0.0f);

    std::cout << "Preparing to copy array of " << num_elements << " elements (~" 
              << (num_elements * sizeof(float)) / (1024 * 1024) << " MB per array).\n";

    // Create a SYCL queue with profiling enabled.
    queue q{ default_selector{}, property::queue::enable_profiling() };
    std::cout << "Running on device: " 
              << q.get_device().get_info<info::device::name>() << "\n";

    // Open log file for writing the kernel times.
    std::ofstream logfile("runtime_log_copy.txt", std::ios::out);
    if (!logfile.is_open()) {
        std::cerr << "Error: Could not open log file for writing." << std::endl;
        return 1;
    }

    // Create buffers for the source and destination arrays.
    {
        buffer<float, 1> bufSrc(src.data(), range<1>(src.size()));
        buffer<float, 1> bufDst(dst.data(), range<1>(dst.size()));

        // Execute the kernel 100 times.
        for (int iter = 0; iter < 1000; iter++) {
            auto event = q.submit([&](handler &h) {
                // Accessors for reading from source and writing to destination.
                auto a = bufSrc.get_access<access::mode::read>(h);
                auto b = bufDst.get_access<access::mode::write>(h);

                // Launch a kernel that copies each element.
                h.parallel_for<class array_copy>(range<1>(num_elements), [=](id<1> idx) {
                    b[idx] = a[idx];
                });
            });

            // Wait for the kernel execution to complete.
            event.wait();

            // Get the kernel's start and end profiling info.
            auto start = event.get_profiling_info<info::event_profiling::command_start>();
            auto end   = event.get_profiling_info<info::event_profiling::command_end>();

            // Calculate elapsed time in nanoseconds.
            auto kernel_time_ns = end - start;

            // Write the iteration and the measured kernel time to the log file.
            logfile << "" << kernel_time_ns << "\n";

            // Optionally print progress.
            std::cout << "Iteration " << iter << " complete; runtime (ns): " 
                      << kernel_time_ns << "\n";
        }
    } // Exiting this scope ensures proper buffer release and data synchronization if needed.

    logfile.close();
    std::cout << "Array copy benchmark complete.\n";

    return 0;
}

import numpy as np
import pyopencl as cl

def main():
    # Set dimensions for matrices (e.g., 1024x1024 for large-scale multiplication)
    M = 10000  # Number of rows in matrix A and C
    K = 10000  # Number of columns in matrix A and rows in matrix B
    N = 10000  # Number of columns in matrix B and C

    # Create two random matrices A and B
    A = np.random.rand(M, K).astype(np.float32)
    B = np.random.rand(K, N).astype(np.float32)
    C = np.empty((M, N), dtype=np.float32)  # Output matrix

    # Set up the OpenCL context, command queue, and device
    platform = cl.get_platforms()[0]  # Select the first platform [change if needed]
    device = platform.get_devices()[0]  # Select the first device [change if needed]
    context = cl.Context([device])
    queue = cl.CommandQueue(context)

    # Create memory buffers on the device for matrices A, B, and C
    mf = cl.mem_flags
    A_buf = cl.Buffer(context, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=A)
    B_buf = cl.Buffer(context, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=B)
    C_buf = cl.Buffer(context, mf.WRITE_ONLY, C.nbytes)

    # Define the OpenCL kernel for matrix multiplication
    kernel_code = """
    __kernel void mat_mul(const int M, const int N, const int K,
                          __global float* A, __global float* B, __global float* C) {
        // Get the row and column index for the element to compute
        int row = get_global_id(0);
        int col = get_global_id(1);
        
        if (row < M && col < N) {
            float sum = 0.0;
            // Perform the dot product of the row of A and column of B
            for (int k = 0; k < K; k++) {
                sum += A[row * K + k] * B[k * N + col];
            }
            // Write the result to the output matrix C
            C[row * N + col] = sum;
        }
    }
    """

    # Build the kernel program
    program = cl.Program(context, kernel_code).build()

    # Define the global work size (each work-item computes one element in C)
    global_work_size = (M, N)

    print("start")
    # Execute the kernel
    program.mat_mul(queue, global_work_size, None,
                    np.int32(M), np.int32(N), np.int32(K),
                    A_buf, B_buf, C_buf).wait()
    print("finish")

    # Copy the result from the device back to the host
    cl.enqueue_copy(queue, C, C_buf)
    queue.finish()

    # Print a small part of the result for verification
    print("Matrix multiplication completed. Result matrix C has shape:", C.shape)
    print("First element C[0, 0] =", C[0, 0])

if __name__ == "__main__":
    main()

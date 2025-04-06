import numpy as np
import pycuda.driver as cuda
import pycuda.autoinit  # Automatically initializes CUDA driver
from pycuda.compiler import SourceModule

def main():
    # Matrix dimensions
    M = 10000  # Number of rows in A and C
    K = 10000  # Number of columns in A and rows in B
    N = 10000  # Number of columns in B and C

    # Create two random matrices A and B
    A = np.random.rand(M, K).astype(np.float32)
    B = np.random.rand(K, N).astype(np.float32)
    C = np.empty((M, N), dtype=np.float32)  # Output matrix

    # Allocate GPU memory and copy data from host to device
    A_gpu = cuda.mem_alloc(A.nbytes)
    B_gpu = cuda.mem_alloc(B.nbytes)
    C_gpu = cuda.mem_alloc(C.nbytes)
    cuda.memcpy_htod(A_gpu, A)
    cuda.memcpy_htod(B_gpu, B)

    # Define the CUDA kernel for matrix multiplication
    kernel_code = """
    __global__ void mat_mul(const int M, const int N, const int K,
                            float *A, float *B, float *C) {
        // Calculate row and column index of the element
        int row = blockIdx.y * blockDim.y + threadIdx.y;
        int col = blockIdx.x * blockDim.x + threadIdx.x;

        if (row < M && col < N) {
            float sum = 0.0;
            // Compute the dot product for element C[row][col]
            for (int k = 0; k < K; k++) {
                sum += A[row * K + k] * B[k * N + col];
            }
            C[row * N + col] = sum;
        }
    }
    """

    # Compile the kernel code
    mod = SourceModule(kernel_code)
    mat_mul = mod.get_function("mat_mul")

    # Define block size and grid size
    block_size = (16, 16, 1)
    grid_size = ((N + block_size[0] - 1) // block_size[0],
                 (M + block_size[1] - 1) // block_size[1])

    # Launch the kernel
    mat_mul(np.int32(M), np.int32(N), np.int32(K),
            A_gpu, B_gpu, C_gpu,
            block=block_size, grid=grid_size)

    # Copy the result from device to host
    cuda.memcpy_dtoh(C, C_gpu)

    # Print a small part of the result for verification
    print("Matrix multiplication completed. Result matrix C has shape:", C.shape)
    print("First element C[0, 0] =", C[0, 0])

if __name__ == "__main__":
    main()

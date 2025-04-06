import numpy as np
import pycuda.driver as cuda
import pycuda.autoinit  # Initializes CUDA automatically
from pycuda.compiler import SourceModule

def main():
    # Define the number of elements in the large array.
    # Here we choose 100 million elements as an example.
    n = 400 * 1024 * 1024  # 100 million elements

    # Create a source array with random data and an empty destination array.
    src_array = np.random.rand(n).astype(np.float32)
    dst_array = np.empty_like(src_array)

    # Allocate device memory for the source and destination arrays.
    src_gpu = cuda.mem_alloc(src_array.nbytes)
    dst_gpu = cuda.mem_alloc(dst_array.nbytes)

    # Copy the source array from host (CPU) to device (GPU).
    cuda.memcpy_htod(src_gpu, src_array)

    # Define the CUDA kernel for copying data.
    kernel_code = """
    __global__ void copy_kernel(const float *src, float *dst, int n) {
        int i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i < n) {
            dst[i] = src[i];
        }
    }
    """

    # Compile the kernel code.
    mod = SourceModule(kernel_code)
    copy_kernel = mod.get_function("copy_kernel")

    # Define the block size and compute the grid size to cover all elements.
    block_size = 256
    grid_size = (n + block_size - 1) // block_size

    # Launch the kernel.
    copy_kernel(src_gpu, dst_gpu, np.int32(n),
                block=(block_size, 1, 1), grid=(grid_size, 1))

    # Copy the result back from device to host.
    cuda.memcpy_dtoh(dst_array, dst_gpu)

    # Verify the copy by comparing arrays.
    if np.allclose(src_array, dst_array):
        print("Array copy successful.")
    else:
        print("Array copy failed.")

if __name__ == "__main__":
    main()

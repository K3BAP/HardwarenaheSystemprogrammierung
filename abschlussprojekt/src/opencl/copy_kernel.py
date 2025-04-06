import numpy as np
import pyopencl as cl

def main():
    # Define the number of elements in the large array.
    n = 600 * 1024 * 1024  # 100 million elements

    # Create a source array with random data and an empty destination array.
    src_array = np.random.rand(n).astype(np.float32)
    # dst_array = np.empty_like(src_array)

    # Set up the OpenCL context, command queue, and device.
    platform = cl.get_platforms()[0]  # Select the first platform (adjust if needed)
    device = platform.get_devices()[0]  # Select the first device (adjust if needed)
    context = cl.Context([device])
    queue = cl.CommandQueue(context)

    # Create memory buffers for the source and destination arrays.
    mf = cl.mem_flags
    src_buf = cl.Buffer(context, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=src_array)
    dst_buf = cl.Buffer(context, mf.WRITE_ONLY, src_array.nbytes)

    # Define the OpenCL kernel for copying data.
    kernel_code = """
    __kernel void copy_kernel(__global const float* src,
                              __global float* dst,
                              const int n) {
        int i = get_global_id(0);
        if (i < n) {
            dst[i] = src[i];
        }
    }
    """

    # Build the OpenCL program.
    program = cl.Program(context, kernel_code).build()

    # Set the global work size to cover all elements.
    global_work_size = (n,)

    print("start")
    # Execute the kernel.
    program.copy_kernel(queue, global_work_size, None, src_buf, dst_buf, np.int32(n)).wait()
    print("finish")

    # Copy the result from the device back to the host.
    # cl.enqueue_copy(queue, dst_array, dst_buf)
    queue.finish()

    # Verify the copy by comparing the arrays.
    #if np.allclose(src_array, dst_array):
    #    print("Array copy successful.")
    #else:
    #    print("Array copy failed.")

if __name__ == "__main__":
    main()

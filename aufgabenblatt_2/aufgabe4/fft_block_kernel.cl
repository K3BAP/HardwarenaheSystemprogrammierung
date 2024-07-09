#pragma OPENCL EXTENSION cl_khr_fp64 : enable

// Perform FFT on a single block of data
__kernel void fft_block(__global float2* input, __global float2* output, int block_size, int step_width, int num_blocks, int total_samples, int inverse) {
    int block_id = get_global_id(0);
    int offset = block_id * step_width;
    int N = block_size;
    
    // Load the block into local memory
    __local float2 block[512];
    for (int i = 0; i < N; ++i) {
        if (offset + i < total_samples) {
            block[i] = input[offset + i];
        } else {
            block[i] = (float2)(0.0f, 0.0f);
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // Perform FFT in local memory
    for (int s = 1; s <= log2((float)N); ++s) {
        int m = 1 << s;
        int m2 = m >> 1;
        float theta = (inverse ? 2 : -2) * M_PI / m;
        float2 w = (float2)(cos(theta), sin(theta));
        float2 w_m = (float2)(1.0f, 0.0f);
        for (int j = 0; j < m2; ++j) {
            for (int k = j; k < N; k += m) {
                int t = k + m2;
                float2 u = block[k];
                float2 v = (float2)(block[t].x * w_m.x - block[t].y * w_m.y, block[t].x * w_m.y + block[t].y * w_m.x);
                block[k] = (float2)(u.x + v.x, u.y + v.y);
                block[t] = (float2)(u.x - v.x, u.y - v.y);
            }
            float temp_wm_x = w_m.x * w.x - w_m.y * w.y;
            w_m.y = w_m.x * w.y + w_m.y * w.x;
            w_m.x = temp_wm_x;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Write the block back to global memory
    for (int i = 0; i < N; ++i) {
        if (offset + i < total_samples) {
            output[block_id * block_size + i] = block[i];
        }
    }
}

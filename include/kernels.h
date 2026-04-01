#ifndef KERNELS_H
#define KERNELS_H

#include <stddef.h>
#include <stdint.h>

void gemm_i32(const int32_t *a, const int32_t *b, int32_t *c, size_t m,
              size_t n, size_t k);
float reduction_sum_f32(const float *input, size_t len);
void sdpa_f32(const float *q, const float *k, const float *v, float *output,
              size_t seq_len, size_t depth, size_t value_dim,
              float *score_scratch, float *weight_scratch);
void run_kernel_demo(void);

#endif // KERNELS_H

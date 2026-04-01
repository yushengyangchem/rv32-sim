#include "kernels.h"
#include <math.h>
#include <stdio.h>

void gemm_i32(const int32_t *a, const int32_t *b, int32_t *c, size_t m,
              size_t n, size_t k) {
  for (size_t row = 0; row < m; row++) {
    for (size_t col = 0; col < n; col++) {
      int32_t acc = 0;
      for (size_t inner = 0; inner < k; inner++) {
        acc += a[row * k + inner] * b[inner * n + col];
      }
      c[row * n + col] = acc;
    }
  }
}

float reduction_sum_f32(const float *input, size_t len) {
  float acc = 0.0f;

  for (size_t i = 0; i < len; i++) {
    acc += input[i];
  }

  return acc;
}

void sdpa_f32(const float *q, const float *k, const float *v, float *output,
              size_t seq_len, size_t depth, size_t value_dim,
              float *score_scratch, float *weight_scratch) {
  const float scale = 1.0f / sqrtf((float)depth);

  for (size_t q_idx = 0; q_idx < seq_len; q_idx++) {
    float max_score = -INFINITY;
    float exp_sum = 0.0f;

    for (size_t k_idx = 0; k_idx < seq_len; k_idx++) {
      float dot = 0.0f;
      for (size_t d = 0; d < depth; d++) {
        dot += q[q_idx * depth + d] * k[k_idx * depth + d];
      }

      score_scratch[k_idx] = dot * scale;
      if (score_scratch[k_idx] > max_score) {
        max_score = score_scratch[k_idx];
      }
    }

    for (size_t k_idx = 0; k_idx < seq_len; k_idx++) {
      float weight = expf(score_scratch[k_idx] - max_score);
      weight_scratch[k_idx] = weight;
      exp_sum += weight;
    }

    for (size_t k_idx = 0; k_idx < seq_len; k_idx++) {
      weight_scratch[k_idx] /= exp_sum;
    }

    for (size_t v_idx = 0; v_idx < value_dim; v_idx++) {
      float acc = 0.0f;
      for (size_t k_idx = 0; k_idx < seq_len; k_idx++) {
        acc += weight_scratch[k_idx] * v[k_idx * value_dim + v_idx];
      }
      output[q_idx * value_dim + v_idx] = acc;
    }
  }
}

static void print_matrix_i32(const char *name, const int32_t *matrix,
                             size_t rows, size_t cols) {
  printf("%s =\n", name);
  for (size_t row = 0; row < rows; row++) {
    printf("  [");
    for (size_t col = 0; col < cols; col++) {
      printf("%d", matrix[row * cols + col]);
      if (col + 1 != cols) {
        printf(", ");
      }
    }
    printf("]\n");
  }
}

void run_kernel_demo(void) {
  const int32_t gemm_a[6] = {
      1, 2, 3, 4, 5, 6,
  };
  const int32_t gemm_b[6] = {
      7, 8, 9, 10, 11, 12,
  };
  int32_t gemm_c[4] = {0};

  const float reduction_input[6] = {1.0f, -2.0f, 3.5f, 4.0f, -1.5f, 2.0f};

  const float q[4] = {
      1.0f,
      0.0f,
      0.0f,
      1.0f,
  };
  const float k[4] = {
      1.0f,
      0.0f,
      0.0f,
      1.0f,
  };
  const float v[4] = {
      1.0f,
      2.0f,
      3.0f,
      4.0f,
  };
  float sdpa_out[4] = {0.0f};
  float score_scratch[2] = {0.0f};
  float weight_scratch[2] = {0.0f};

  printf("========================================\n");
  printf(" Core Kernel Demo (Software Reference)  \n");
  printf("========================================\n");

  gemm_i32(gemm_a, gemm_b, gemm_c, 2, 2, 3);
  print_matrix_i32("GeMM A", gemm_a, 2, 3);
  print_matrix_i32("GeMM B", gemm_b, 3, 2);
  print_matrix_i32("GeMM C = A x B", gemm_c, 2, 2);

  printf("\nReduction sum = %.3f\n", reduction_sum_f32(reduction_input, 6));

  sdpa_f32(q, k, v, sdpa_out, 2, 2, 2, score_scratch, weight_scratch);
  printf("\nSDPA output =\n");
  printf("  [%.4f, %.4f]\n", sdpa_out[0], sdpa_out[1]);
  printf("  [%.4f, %.4f]\n", sdpa_out[2], sdpa_out[3]);
}

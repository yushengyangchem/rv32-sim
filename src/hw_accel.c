#include "hw_accel.h"
#include "kernels.h"
#include "memory.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REDUCTION_DEMO_LEN 6u
#define SDPA_DEMO_SEQ_LEN 2u
#define SDPA_DEMO_DEPTH 2u
#define SDPA_DEMO_VALUE_DIM 2u

static float read_f32(uint32_t addr) {
  uint32_t bits = mem_read_32(addr);
  float value = 0.0f;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

static void write_f32(uint32_t addr, float value) {
  uint32_t bits = 0;
  memcpy(&bits, &value, sizeof(bits));
  mem_write_32(addr, bits);
}

static void load_matrix_from_mem(uint32_t base_addr, int32_t *matrix,
                                 size_t count) {
  for (size_t i = 0; i < count; i++) {
    matrix[i] =
        (int32_t)mem_read_32(base_addr + (uint32_t)(i * sizeof(uint32_t)));
  }
}

static void store_matrix_to_mem(uint32_t base_addr, const int32_t *matrix,
                                size_t count) {
  for (size_t i = 0; i < count; i++) {
    mem_write_32(base_addr + (uint32_t)(i * sizeof(uint32_t)),
                 (uint32_t)matrix[i]);
  }
}

static bool multiply_would_overflow_size(size_t a, size_t b) {
  return a != 0u && b > (SIZE_MAX / a);
}

static bool address_range_valid(uint32_t base_addr, size_t count,
                                size_t elem_size) {
  size_t total_bytes = 0;

  if (count == 0u) {
    return true;
  }

  if (multiply_would_overflow_size(count, elem_size)) {
    return false;
  }

  total_bytes = count * elem_size;
  if ((size_t)base_addr >= MEMORY_SIZE) {
    return false;
  }

  return total_bytes <= (MEMORY_SIZE - (size_t)base_addr);
}

static void print_matrix(const char *name, const int32_t *matrix, size_t rows,
                         size_t cols) {
  printf("[HW ACCEL] %s =\n", name);
  for (size_t row = 0; row < rows; row++) {
    printf("[HW ACCEL]   [");
    for (size_t col = 0; col < cols; col++) {
      printf("%d", matrix[row * cols + col]);
      if (col + 1 != cols) {
        printf(", ");
      }
    }
    printf("]\n");
  }
}

static void print_vector_f32(const char *name, const float *vector,
                             size_t len) {
  printf("[HW ACCEL] %s = [", name);
  for (size_t i = 0; i < len; i++) {
    printf("%.4f", vector[i]);
    if (i + 1 != len) {
      printf(", ");
    }
  }
  printf("]\n");
}

static void print_matrix_f32(const char *name, const float *matrix, size_t rows,
                             size_t cols) {
  printf("[HW ACCEL] %s =\n", name);
  for (size_t row = 0; row < rows; row++) {
    printf("[HW ACCEL]   [");
    for (size_t col = 0; col < cols; col++) {
      printf("%.4f", matrix[row * cols + col]);
      if (col + 1 != cols) {
        printf(", ");
      }
    }
    printf("]\n");
  }
}

const char *hw_accel_status_name(uint32_t status) {
  switch (status) {
  case HW_ACCEL_STATUS_OK:
    return "OK";
  case HW_ACCEL_STATUS_ERR_ZERO_LENGTH:
    return "ERR_ZERO_LENGTH";
  case HW_ACCEL_STATUS_ERR_ZERO_DIMENSION:
    return "ERR_ZERO_DIMENSION";
  case HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW:
    return "ERR_SIZE_OVERFLOW";
  case HW_ACCEL_STATUS_ERR_ADDRESS_RANGE:
    return "ERR_ADDRESS_RANGE";
  case HW_ACCEL_STATUS_ERR_ALLOCATION:
    return "ERR_ALLOCATION";
  default:
    return "ERR_UNKNOWN";
  }
}

void hw_accel_init_demo_data(void) {
  const uint32_t gemm_rows = 2u;
  const uint32_t gemm_cols = 2u;
  const uint32_t gemm_depth = 3u;
  const int32_t matrix_a[] = {
      1, 2, 3, 4, 5, 6,
  };
  const int32_t matrix_b[] = {
      7, 8, 9, 10, 11, 12,
  };
  const float reduction_input[REDUCTION_DEMO_LEN] = {
      1.0f, -2.0f, 3.5f, 4.0f, -1.5f, 2.0f,
  };
  const float q[SDPA_DEMO_SEQ_LEN * SDPA_DEMO_DEPTH] = {
      1.0f,
      0.0f,
      0.0f,
      1.0f,
  };
  const float k[SDPA_DEMO_SEQ_LEN * SDPA_DEMO_DEPTH] = {
      1.0f,
      0.0f,
      0.0f,
      1.0f,
  };
  const float v[SDPA_DEMO_SEQ_LEN * SDPA_DEMO_VALUE_DIM] = {
      1.0f,
      2.0f,
      3.0f,
      4.0f,
  };

  store_matrix_to_mem(HW_ACCEL_GEMM_DEMO_A_ADDR, matrix_a,
                      (size_t)gemm_rows * gemm_depth);
  store_matrix_to_mem(HW_ACCEL_GEMM_DEMO_B_ADDR, matrix_b,
                      (size_t)gemm_depth * gemm_cols);

  for (size_t i = 0; i < (size_t)gemm_rows * gemm_cols; i++) {
    mem_write_32(HW_ACCEL_GEMM_DEMO_C_ADDR + (uint32_t)(i * sizeof(uint32_t)),
                 0);
  }
  mem_write_32(HW_ACCEL_GEMM_DEMO_DESC_ADDR, HW_ACCEL_GEMM_DEMO_B_ADDR);
  mem_write_32(HW_ACCEL_GEMM_DEMO_DESC_ADDR + 4u, HW_ACCEL_GEMM_DEMO_C_ADDR);
  mem_write_32(HW_ACCEL_GEMM_DEMO_DESC_ADDR + 8u, gemm_rows);
  mem_write_32(HW_ACCEL_GEMM_DEMO_DESC_ADDR + 12u, gemm_cols);
  mem_write_32(HW_ACCEL_GEMM_DEMO_DESC_ADDR + 16u, gemm_depth);

  for (size_t i = 0; i < REDUCTION_DEMO_LEN; i++) {
    write_f32(HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR +
                  (uint32_t)(i * sizeof(float)),
              reduction_input[i]);
  }
  mem_write_32(HW_ACCEL_REDUCTION_DEMO_DESC_ADDR, REDUCTION_DEMO_LEN);
  mem_write_32(HW_ACCEL_REDUCTION_DEMO_DESC_ADDR + 4u,
               HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR);
  write_f32(HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR, 0.0f);

  for (size_t i = 0; i < SDPA_DEMO_SEQ_LEN * SDPA_DEMO_DEPTH; i++) {
    write_f32(HW_ACCEL_SDPA_DEMO_Q_ADDR + (uint32_t)(i * sizeof(float)), q[i]);
    write_f32(HW_ACCEL_SDPA_DEMO_K_ADDR + (uint32_t)(i * sizeof(float)), k[i]);
  }
  for (size_t i = 0; i < SDPA_DEMO_SEQ_LEN * SDPA_DEMO_VALUE_DIM; i++) {
    write_f32(HW_ACCEL_SDPA_DEMO_V_ADDR + (uint32_t)(i * sizeof(float)), v[i]);
    write_f32(HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + (uint32_t)(i * sizeof(float)),
              0.0f);
  }

  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR, HW_ACCEL_SDPA_DEMO_K_ADDR);
  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR + 4u, HW_ACCEL_SDPA_DEMO_V_ADDR);
  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR + 8u,
               HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR);
  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR + 12u, SDPA_DEMO_SEQ_LEN);
  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR + 16u, SDPA_DEMO_DEPTH);
  mem_write_32(HW_ACCEL_SDPA_DEMO_DESC_ADDR + 20u, SDPA_DEMO_VALUE_DIM);
}

uint32_t hw_accel_gemm(uint32_t matrix_a_addr, uint32_t desc_addr) {
  GemmDescriptor desc;
  int32_t *matrix_a = NULL;
  int32_t *matrix_b = NULL;
  int32_t *matrix_c = NULL;
  size_t a_count = 0;
  size_t b_count = 0;
  size_t c_count = 0;

  desc.b_addr = mem_read_32(desc_addr);
  desc.output_addr = mem_read_32(desc_addr + 4u);
  desc.rows = mem_read_32(desc_addr + 8u);
  desc.cols = mem_read_32(desc_addr + 12u);
  desc.depth = mem_read_32(desc_addr + 16u);

  if (desc.rows == 0u || desc.cols == 0u || desc.depth == 0u) {
    printf("[HW ACCEL] GeMM descriptor has zero dimension.\n");
    return HW_ACCEL_STATUS_ERR_ZERO_DIMENSION;
  }

  if (multiply_would_overflow_size(desc.rows, desc.depth) ||
      multiply_would_overflow_size(desc.depth, desc.cols) ||
      multiply_would_overflow_size(desc.rows, desc.cols)) {
    printf("[HW ACCEL] GeMM descriptor dimensions overflow host size_t.\n");
    return HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW;
  }

  a_count = (size_t)desc.rows * desc.depth;
  b_count = (size_t)desc.depth * desc.cols;
  c_count = (size_t)desc.rows * desc.cols;

  if (!address_range_valid(matrix_a_addr, a_count, sizeof(int32_t)) ||
      !address_range_valid(desc.b_addr, b_count, sizeof(int32_t)) ||
      !address_range_valid(desc.output_addr, c_count, sizeof(int32_t))) {
    printf("[HW ACCEL] GeMM descriptor points outside simulated memory.\n");
    return HW_ACCEL_STATUS_ERR_ADDRESS_RANGE;
  }

  matrix_a = malloc(a_count * sizeof(int32_t));
  matrix_b = malloc(b_count * sizeof(int32_t));
  matrix_c = malloc(c_count * sizeof(int32_t));
  if (!matrix_a || !matrix_b || !matrix_c) {
    printf("[HW ACCEL] GeMM allocation failed.\n");
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
    return HW_ACCEL_STATUS_ERR_ALLOCATION;
  }

  load_matrix_from_mem(matrix_a_addr, matrix_a, a_count);
  load_matrix_from_mem(desc.b_addr, matrix_b, b_count);
  gemm_i32(matrix_a, matrix_b, matrix_c, desc.rows, desc.cols, desc.depth);
  store_matrix_to_mem(desc.output_addr, matrix_c, c_count);

  printf("\n==================================================\n");
  printf("[HW ACCEL] Custom GeMM Instruction Triggered!\n");
  printf("[HW ACCEL] Matrix A Base Address: 0x%08X\n", matrix_a_addr);
  printf("[HW ACCEL] Descriptor Address: 0x%08X\n", desc_addr);
  printf("[HW ACCEL] Matrix B Base Address: 0x%08X\n", desc.b_addr);
  printf("[HW ACCEL] Output Base Address: 0x%08X\n", desc.output_addr);
  printf("[HW ACCEL] Shape: rows=%u cols=%u depth=%u\n", desc.rows, desc.cols,
         desc.depth);
  print_matrix("Matrix A", matrix_a, desc.rows, desc.depth);
  print_matrix("Matrix B", matrix_b, desc.depth, desc.cols);
  print_matrix("Result C", matrix_c, desc.rows, desc.cols);
  printf("[HW ACCEL] Result written back to: 0x%08X\n", desc.output_addr);
  printf("[HW ACCEL] GeMM computation simulated successfully.\n");
  printf("==================================================\n\n");

  free(matrix_a);
  free(matrix_b);
  free(matrix_c);
  return HW_ACCEL_STATUS_OK;
}

uint32_t hw_accel_reduction(uint32_t input_addr, uint32_t desc_addr) {
  ReductionDescriptor desc;
  float *input = NULL;
  float result = 0.0f;

  desc.len = mem_read_32(desc_addr);
  desc.output_addr = mem_read_32(desc_addr + 4u);

  if (desc.len == 0u) {
    printf("[HW ACCEL] Reduction descriptor has zero length.\n");
    return HW_ACCEL_STATUS_ERR_ZERO_LENGTH;
  }

  if (!address_range_valid(input_addr, desc.len, sizeof(float)) ||
      !address_range_valid(desc.output_addr, 1u, sizeof(float))) {
    printf(
        "[HW ACCEL] Reduction descriptor points outside simulated memory.\n");
    return HW_ACCEL_STATUS_ERR_ADDRESS_RANGE;
  }

  input = malloc(desc.len * sizeof(float));
  if (!input) {
    printf("[HW ACCEL] Reduction allocation failed.\n");
    return HW_ACCEL_STATUS_ERR_ALLOCATION;
  }

  for (size_t i = 0; i < desc.len; i++) {
    input[i] = read_f32(input_addr + (uint32_t)(i * sizeof(float)));
  }

  result = reduction_sum_f32(input, desc.len);
  write_f32(desc.output_addr, result);

  printf("\n==================================================\n");
  printf("[HW ACCEL] Custom Reduction Instruction Triggered!\n");
  printf("[HW ACCEL] Input Base Address: 0x%08X\n", input_addr);
  printf("[HW ACCEL] Descriptor Address: 0x%08X\n", desc_addr);
  print_vector_f32("Reduction Input", input, desc.len);
  printf("[HW ACCEL] Reduction Result = %.4f\n", result);
  printf("[HW ACCEL] Result written back to: 0x%08X\n", desc.output_addr);
  printf("[HW ACCEL] Reduction computation simulated successfully.\n");
  printf("==================================================\n\n");

  free(input);
  return HW_ACCEL_STATUS_OK;
}

uint32_t hw_accel_sdpa(uint32_t q_addr, uint32_t desc_addr) {
  SdpaDescriptor desc;
  float *q = NULL;
  float *k = NULL;
  float *v = NULL;
  float *output = NULL;
  float *score_scratch = NULL;
  float *weight_scratch = NULL;
  size_t qk_count = 0;
  size_t v_count = 0;

  desc.k_addr = mem_read_32(desc_addr);
  desc.v_addr = mem_read_32(desc_addr + 4u);
  desc.output_addr = mem_read_32(desc_addr + 8u);
  desc.seq_len = mem_read_32(desc_addr + 12u);
  desc.depth = mem_read_32(desc_addr + 16u);
  desc.value_dim = mem_read_32(desc_addr + 20u);

  if (desc.seq_len == 0u || desc.depth == 0u || desc.value_dim == 0u) {
    printf("[HW ACCEL] SDPA descriptor has zero dimension.\n");
    return HW_ACCEL_STATUS_ERR_ZERO_DIMENSION;
  }

  if (multiply_would_overflow_size(desc.seq_len, desc.depth) ||
      multiply_would_overflow_size(desc.seq_len, desc.value_dim)) {
    printf("[HW ACCEL] SDPA descriptor dimensions overflow host size_t.\n");
    return HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW;
  }

  qk_count = (size_t)desc.seq_len * desc.depth;
  v_count = (size_t)desc.seq_len * desc.value_dim;

  if (!address_range_valid(q_addr, qk_count, sizeof(float)) ||
      !address_range_valid(desc.k_addr, qk_count, sizeof(float)) ||
      !address_range_valid(desc.v_addr, v_count, sizeof(float)) ||
      !address_range_valid(desc.output_addr, v_count, sizeof(float))) {
    printf("[HW ACCEL] SDPA descriptor points outside simulated memory.\n");
    return HW_ACCEL_STATUS_ERR_ADDRESS_RANGE;
  }

  q = malloc(qk_count * sizeof(float));
  k = malloc(qk_count * sizeof(float));
  v = malloc(v_count * sizeof(float));
  output = malloc(v_count * sizeof(float));
  score_scratch = malloc(desc.seq_len * sizeof(float));
  weight_scratch = malloc(desc.seq_len * sizeof(float));
  if (!q || !k || !v || !output || !score_scratch || !weight_scratch) {
    printf("[HW ACCEL] SDPA allocation failed.\n");
    free(q);
    free(k);
    free(v);
    free(output);
    free(score_scratch);
    free(weight_scratch);
    return HW_ACCEL_STATUS_ERR_ALLOCATION;
  }

  for (size_t i = 0; i < qk_count; i++) {
    q[i] = read_f32(q_addr + (uint32_t)(i * sizeof(float)));
    k[i] = read_f32(desc.k_addr + (uint32_t)(i * sizeof(float)));
  }
  for (size_t i = 0; i < v_count; i++) {
    v[i] = read_f32(desc.v_addr + (uint32_t)(i * sizeof(float)));
  }

  sdpa_f32(q, k, v, output, desc.seq_len, desc.depth, desc.value_dim,
           score_scratch, weight_scratch);

  for (size_t i = 0; i < v_count; i++) {
    write_f32(desc.output_addr + (uint32_t)(i * sizeof(float)), output[i]);
  }

  printf("\n==================================================\n");
  printf("[HW ACCEL] Custom SDPA Instruction Triggered!\n");
  printf("[HW ACCEL] Q Base Address: 0x%08X\n", q_addr);
  printf("[HW ACCEL] Descriptor Address: 0x%08X\n", desc_addr);
  print_matrix_f32("Q", q, desc.seq_len, desc.depth);
  print_matrix_f32("K", k, desc.seq_len, desc.depth);
  print_matrix_f32("V", v, desc.seq_len, desc.value_dim);
  print_matrix_f32("SDPA Output", output, desc.seq_len, desc.value_dim);
  printf("[HW ACCEL] Result written back to: 0x%08X\n", desc.output_addr);
  printf("[HW ACCEL] SDPA computation simulated successfully.\n");
  printf("==================================================\n\n");

  free(q);
  free(k);
  free(v);
  free(output);
  free(score_scratch);
  free(weight_scratch);
  return HW_ACCEL_STATUS_OK;
}

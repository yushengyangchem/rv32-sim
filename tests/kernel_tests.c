#include "kernels.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static bool nearly_equal(float actual, float expected, float tolerance) {
  float diff = fabsf(actual - expected);
  return diff <= tolerance;
}

static bool test_gemm_reference(void) {
  const int32_t a[6] = {
      1, 2, 3, 4, 5, 6,
  };
  const int32_t b[6] = {
      7, 8, 9, 10, 11, 12,
  };
  const int32_t expected[4] = {
      58,
      64,
      139,
      154,
  };
  int32_t actual[4] = {0};

  gemm_i32(a, b, actual, 2, 2, 3);

  for (size_t i = 0; i < 4; i++) {
    if (actual[i] != expected[i]) {
      printf(
          "[FAIL] GeMM reference mismatch at index %zu: got %d, expected %d\n",
          i, actual[i], expected[i]);
      return false;
    }
  }

  printf("[PASS] GeMM reference\n");
  return true;
}

static bool test_reduction_reference(void) {
  const float input[6] = {1.0f, -2.0f, 3.5f, 4.0f, -1.5f, 2.0f};
  const float expected = 7.0f;
  const float actual = reduction_sum_f32(input, 6);

  if (!nearly_equal(actual, expected, 1e-6f)) {
    printf("[FAIL] Reduction mismatch: got %.6f, expected %.6f\n", actual,
           expected);
    return false;
  }

  printf("[PASS] Reduction reference\n");
  return true;
}

static bool test_sdpa_reference(void) {
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
  const float expected[4] = {
      1.6604769f,
      2.6604769f,
      2.3395228f,
      3.3395228f,
  };
  float output[4] = {0.0f};
  float score_scratch[2] = {0.0f};
  float weight_scratch[2] = {0.0f};

  sdpa_f32(q, k, v, output, 2, 2, 2, score_scratch, weight_scratch);

  for (size_t i = 0; i < 4; i++) {
    if (!nearly_equal(output[i], expected[i], 1e-4f)) {
      printf("[FAIL] SDPA mismatch at index %zu: got %.6f, expected %.6f\n", i,
             output[i], expected[i]);
      return false;
    }
  }

  printf("[PASS] SDPA reference\n");
  return true;
}

int main(void) {
  bool ok = true;

  ok = test_gemm_reference() && ok;
  ok = test_reduction_reference() && ok;
  ok = test_sdpa_reference() && ok;

  if (!ok) {
    printf("[TEST] kernel tests failed\n");
    return 1;
  }

  printf("[TEST] all kernel tests passed\n");
  return 0;
}

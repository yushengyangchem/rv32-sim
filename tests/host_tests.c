#include "accel_layout.h"
#include "hw_accel.h"
#include "memory.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool nearly_equal(float actual, float expected, float tolerance) {
  float diff = fabsf(actual - expected);
  return diff <= tolerance;
}

static bool test_hw_accel_gemm(void) {
  const int32_t expected[4] = {
      58,
      64,
      139,
      154,
  };

  mem_reset();
  if (hw_accel_init_demo_data() != 0) {
    printf("[FAIL] hw_accel_init_demo_data failed\n");
    return false;
  }

  if (hw_accel_gemm(HW_ACCEL_GEMM_DEMO_A_ADDR, HW_ACCEL_GEMM_DEMO_DESC_ADDR) !=
      HW_ACCEL_STATUS_OK) {
    printf("[FAIL] HW accelerator returned non-success status\n");
    return false;
  }

  for (size_t i = 0; i < 4; i++) {
    uint32_t addr =
        HW_ACCEL_GEMM_DEMO_C_ADDR + (uint32_t)(i * sizeof(uint32_t));
    uint32_t val = 0;
    if (mem_read_32(addr, &val) != 0) {
      printf("[FAIL] mem_read_32 error at 0x%08X\n", addr);
      return false;
    }
    int32_t actual = (int32_t)val;
    if (actual != expected[i]) {
      printf("[FAIL] HW accelerator result mismatch at index %zu: got %d, "
             "expected %d\n",
             i, actual, expected[i]);
      return false;
    }
  }

  printf("[PASS] HW accelerator GeMM behavioral model\n");
  return true;
}

static bool test_hw_accel_gemm_variable_shape(void) {
  const uint32_t matrix_a_addr = 0x00006000u;
  const uint32_t matrix_b_addr = 0x00006100u;
  const uint32_t matrix_c_addr = 0x00006200u;
  const uint32_t desc_addr = 0x00006300u;
  const int32_t matrix_a[6] = {
      1, 2, 3, 4, 5, 6,
  };
  const int32_t matrix_b[6] = {
      7, 8, 9, 10, 11, 12,
  };
  const int32_t expected[4] = {
      58,
      64,
      139,
      154,
  };

  mem_reset();

  for (size_t i = 0; i < 6; i++) {
    uint32_t a_addr = matrix_a_addr + (uint32_t)(i * sizeof(uint32_t));
    uint32_t b_addr = matrix_b_addr + (uint32_t)(i * sizeof(uint32_t));
    if (mem_write_32(a_addr, (uint32_t)matrix_a[i]) != 0) {
      printf("[FAIL] mem_write_32 error at 0x%08X\n", a_addr);
      return false;
    }
    if (mem_write_32(b_addr, (uint32_t)matrix_b[i]) != 0) {
      printf("[FAIL] mem_write_32 error at 0x%08X\n", b_addr);
      return false;
    }
  }

  if (mem_write_32(desc_addr, matrix_b_addr) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, matrix_c_addr) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }
  if (mem_write_32(desc_addr + 8u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 8u);
    return false;
  }
  if (mem_write_32(desc_addr + 12u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 12u);
    return false;
  }
  if (mem_write_32(desc_addr + 16u, 3u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 16u);
    return false;
  }

  if (hw_accel_gemm(matrix_a_addr, desc_addr) != HW_ACCEL_STATUS_OK) {
    printf(
        "[FAIL] Variable-shape GeMM accelerator returned non-success status\n");
    return false;
  }

  for (size_t i = 0; i < 4; i++) {
    uint32_t addr = matrix_c_addr + (uint32_t)(i * sizeof(uint32_t));
    uint32_t val = 0;
    if (mem_read_32(addr, &val) != 0) {
      printf("[FAIL] mem_read_32 error at 0x%08X\n", addr);
      return false;
    }
    int32_t actual = (int32_t)val;
    if (actual != expected[i]) {
      printf("[FAIL] Variable-shape GeMM mismatch at index %zu: got %d, "
             "expected %d\n",
             i, actual, expected[i]);
      return false;
    }
  }

  printf("[PASS] HW accelerator GeMM variable-shape descriptor path\n");
  return true;
}

static bool test_hw_accel_reduction(void) {
  const float expected = 7.0f;
  float actual = 0.0f;

  mem_reset();
  if (hw_accel_init_demo_data() != 0) {
    printf("[FAIL] hw_accel_init_demo_data failed\n");
    return false;
  }

  if (hw_accel_reduction(HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR,
                         HW_ACCEL_REDUCTION_DEMO_DESC_ADDR) !=
      HW_ACCEL_STATUS_OK) {
    printf("[FAIL] Reduction accelerator returned non-success status\n");
    return false;
  }

  {
    uint32_t bits = 0;
    if (mem_read_32(HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR, &bits) != 0) {
      printf("[FAIL] mem_read_32 error at 0x%08X\n",
             HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR);
      return false;
    }
    memcpy(&actual, &bits, sizeof(actual));
  }

  if (!nearly_equal(actual, expected, 1e-6f)) {
    printf("[FAIL] Reduction accelerator mismatch: got %.6f, expected %.6f\n",
           actual, expected);
    return false;
  }

  printf("[PASS] HW accelerator Reduction behavioral model\n");
  return true;
}

static bool test_hw_accel_reduction_zero_length_rejected(void) {
  const uint32_t input_addr = 0x00007000u;
  const uint32_t desc_addr = 0x00007100u;
  const uint32_t output_addr = 0x00007200u;

  mem_reset();
  if (mem_write_32(desc_addr, 0u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, output_addr) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }
  if (mem_write_32(output_addr, 0xDEADBEEFu) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", output_addr);
    return false;
  }

  if (hw_accel_reduction(input_addr, desc_addr) !=
      HW_ACCEL_STATUS_ERR_ZERO_LENGTH) {
    printf("[FAIL] Reduction zero-length descriptor should be rejected\n");
    return false;
  }

  {
    uint32_t val = 0;
    if (mem_read_32(output_addr, &val) != 0) {
      printf("[FAIL] mem_read_32 error at 0x%08X\n", output_addr);
      return false;
    }
    if (val != 0xDEADBEEFu) {
      printf(
          "[FAIL] Reduction zero-length descriptor should not modify output\n");
      return false;
    }
  }

  printf("[PASS] Reduction zero-length descriptor rejected\n");
  return true;
}

static bool test_hw_accel_reduction_invalid_output_rejected(void) {
  const uint32_t input_addr = 0x00007300u;
  const uint32_t desc_addr = 0x00007400u;

  mem_reset();
  if (mem_write_32(input_addr, 0x3F800000u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", input_addr);
    return false;
  }
  if (mem_write_32(desc_addr, 1u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, MEMORY_SIZE) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }

  if (hw_accel_reduction(input_addr, desc_addr) !=
      HW_ACCEL_STATUS_ERR_ADDRESS_RANGE) {
    printf("[FAIL] Reduction invalid output address should be rejected\n");
    return false;
  }

  printf("[PASS] Reduction invalid output address rejected\n");
  return true;
}

static bool test_hw_accel_sdpa(void) {
  const float expected[4] = {
      1.6604769f,
      2.6604769f,
      2.3395228f,
      3.3395228f,
  };

  mem_reset();
  if (hw_accel_init_demo_data() != 0) {
    printf("[FAIL] hw_accel_init_demo_data failed\n");
    return false;
  }

  if (hw_accel_sdpa(HW_ACCEL_SDPA_DEMO_Q_ADDR, HW_ACCEL_SDPA_DEMO_DESC_ADDR) !=
      HW_ACCEL_STATUS_OK) {
    printf("[FAIL] SDPA accelerator returned non-success status\n");
    return false;
  }

  for (size_t i = 0; i < 4; i++) {
    float actual = 0.0f;
    uint32_t addr =
        HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + (uint32_t)(i * sizeof(float));
    uint32_t bits = 0;
    if (mem_read_32(addr, &bits) != 0) {
      printf("[FAIL] mem_read_32 error at 0x%08X\n", addr);
      return false;
    }
    memcpy(&actual, &bits, sizeof(actual));
    if (!nearly_equal(actual, expected[i], 1e-4f)) {
      printf("[FAIL] SDPA accelerator mismatch at index %zu: got %.6f, "
             "expected %.6f\n",
             i, actual, expected[i]);
      return false;
    }
  }

  printf("[PASS] HW accelerator SDPA behavioral model\n");
  return true;
}

static bool test_hw_accel_sdpa_zero_dimension_rejected(void) {
  const uint32_t q_addr = 0x00008000u;
  const uint32_t desc_addr = 0x00008100u;

  mem_reset();
  if (mem_write_32(desc_addr, 0x00008200u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, 0x00008300u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }
  if (mem_write_32(desc_addr + 8u, 0x00008400u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 8u);
    return false;
  }
  if (mem_write_32(desc_addr + 12u, 0u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 12u);
    return false;
  }
  if (mem_write_32(desc_addr + 16u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 16u);
    return false;
  }
  if (mem_write_32(desc_addr + 20u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 20u);
    return false;
  }

  if (hw_accel_sdpa(q_addr, desc_addr) != HW_ACCEL_STATUS_ERR_ZERO_DIMENSION) {
    printf("[FAIL] SDPA zero-dimension descriptor should be rejected\n");
    return false;
  }

  printf("[PASS] SDPA zero-dimension descriptor rejected\n");
  return true;
}

static bool test_hw_accel_sdpa_invalid_output_rejected(void) {
  const uint32_t q_addr = 0x00008500u;
  const uint32_t k_addr = 0x00008600u;
  const uint32_t v_addr = 0x00008700u;
  const uint32_t desc_addr = 0x00008800u;

  mem_reset();

  for (size_t i = 0; i < 4; i++) {
    uint32_t qa = q_addr + (uint32_t)(i * sizeof(uint32_t));
    uint32_t ka = k_addr + (uint32_t)(i * sizeof(uint32_t));
    uint32_t va = v_addr + (uint32_t)(i * sizeof(uint32_t));
    if (mem_write_32(qa, 0x3F800000u) != 0) {
      printf("[FAIL] mem_write_32 error at 0x%08X\n", qa);
      return false;
    }
    if (mem_write_32(ka, 0x3F800000u) != 0) {
      printf("[FAIL] mem_write_32 error at 0x%08X\n", ka);
      return false;
    }
    if (mem_write_32(va, 0x3F800000u) != 0) {
      printf("[FAIL] mem_write_32 error at 0x%08X\n", va);
      return false;
    }
  }

  if (mem_write_32(desc_addr, k_addr) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, v_addr) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }
  if (mem_write_32(desc_addr + 8u, MEMORY_SIZE) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 8u);
    return false;
  }
  if (mem_write_32(desc_addr + 12u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 12u);
    return false;
  }
  if (mem_write_32(desc_addr + 16u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 16u);
    return false;
  }
  if (mem_write_32(desc_addr + 20u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 20u);
    return false;
  }

  if (hw_accel_sdpa(q_addr, desc_addr) != HW_ACCEL_STATUS_ERR_ADDRESS_RANGE) {
    printf("[FAIL] SDPA invalid output address should be rejected\n");
    return false;
  }

  printf("[PASS] SDPA invalid output address rejected\n");
  return true;
}

static bool test_hw_accel_sdpa_dimension_overflow_rejected(void) {
  const uint32_t q_addr = 0x00008900u;
  const uint32_t desc_addr = 0x00008A00u;

  mem_reset();
  if (mem_write_32(desc_addr, 0x00008B00u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr);
    return false;
  }
  if (mem_write_32(desc_addr + 4u, 0x00008C00u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 4u);
    return false;
  }
  if (mem_write_32(desc_addr + 8u, 0x00008D00u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 8u);
    return false;
  }
  if (mem_write_32(desc_addr + 12u, 0xFFFFFFFFu) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 12u);
    return false;
  }
  if (mem_write_32(desc_addr + 16u, 0xFFFFFFFFu) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 16u);
    return false;
  }
  if (mem_write_32(desc_addr + 20u, 2u) != 0) {
    printf("[FAIL] mem_write_32 error at 0x%08X\n", desc_addr + 20u);
    return false;
  }

  if (hw_accel_sdpa(q_addr, desc_addr) != HW_ACCEL_STATUS_ERR_ADDRESS_RANGE) {
    printf("[FAIL] SDPA overflowing dimensions should be rejected\n");
    return false;
  }

  printf("[PASS] SDPA overflowing dimensions rejected\n");
  return true;
}

int main(void) {
  bool ok = true;

  ok = test_hw_accel_gemm() && ok;
  ok = test_hw_accel_gemm_variable_shape() && ok;
  ok = test_hw_accel_reduction() && ok;
  ok = test_hw_accel_reduction_zero_length_rejected() && ok;
  ok = test_hw_accel_reduction_invalid_output_rejected() && ok;
  ok = test_hw_accel_sdpa() && ok;
  ok = test_hw_accel_sdpa_zero_dimension_rejected() && ok;
  ok = test_hw_accel_sdpa_invalid_output_rejected() && ok;
  ok = test_hw_accel_sdpa_dimension_overflow_rejected() && ok;

  if (!ok) {
    printf("[TEST] host accelerator tests failed\n");
    return 1;
  }

  printf("[TEST] all host accelerator tests passed\n");
  return 0;
}

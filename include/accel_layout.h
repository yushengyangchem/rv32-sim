#ifndef ACCEL_LAYOUT_H
#define ACCEL_LAYOUT_H

#include <stdint.h>

#define HW_ACCEL_CUSTOM_FUNCT3_GEMM 0u
#define HW_ACCEL_CUSTOM_FUNCT3_REDUCTION 1u
#define HW_ACCEL_CUSTOM_FUNCT3_SDPA 2u

#define HW_ACCEL_GEMM_DEMO_A_ADDR 0x00080000u
#define HW_ACCEL_GEMM_DEMO_B_ADDR 0x00081000u
#define HW_ACCEL_GEMM_DEMO_C_ADDR 0x00082000u
#define HW_ACCEL_GEMM_DEMO_DESC_ADDR 0x00082100u

#define HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR 0x00083000u
#define HW_ACCEL_REDUCTION_DEMO_DESC_ADDR 0x00083100u
#define HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR 0x00083200u

#define HW_ACCEL_SDPA_DEMO_Q_ADDR 0x00084000u
#define HW_ACCEL_SDPA_DEMO_DESC_ADDR 0x00084100u
#define HW_ACCEL_SDPA_DEMO_K_ADDR 0x00084200u
#define HW_ACCEL_SDPA_DEMO_V_ADDR 0x00084300u
#define HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR 0x00084400u

typedef struct {
  uint32_t b_addr;
  uint32_t output_addr;
  uint32_t rows;
  uint32_t cols;
  uint32_t depth;
} GemmDescriptor;

typedef struct {
  uint32_t len;
  uint32_t output_addr;
} ReductionDescriptor;

typedef struct {
  uint32_t k_addr;
  uint32_t v_addr;
  uint32_t output_addr;
  uint32_t seq_len;
  uint32_t depth;
  uint32_t value_dim;
} SdpaDescriptor;

#endif // ACCEL_LAYOUT_H

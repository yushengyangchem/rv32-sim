#ifndef ACCEL_LAYOUT_H
#define ACCEL_LAYOUT_H

#include <stdint.h>

#define HW_ACCEL_CUSTOM_FUNCT3_GEMM 0u
#define HW_ACCEL_CUSTOM_FUNCT3_REDUCTION 1u
#define HW_ACCEL_CUSTOM_FUNCT3_SDPA 2u

#define HW_ACCEL_GEMM_DEMO_A_ADDR 0x00001000u
#define HW_ACCEL_GEMM_DEMO_B_ADDR 0x00002000u
#define HW_ACCEL_GEMM_DEMO_C_ADDR 0x00003000u
#define HW_ACCEL_GEMM_DEMO_DESC_ADDR 0x00003100u

#define HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR 0x00004000u
#define HW_ACCEL_REDUCTION_DEMO_DESC_ADDR 0x00004100u
#define HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR 0x00004200u

#define HW_ACCEL_SDPA_DEMO_Q_ADDR 0x00005000u
#define HW_ACCEL_SDPA_DEMO_DESC_ADDR 0x00005100u
#define HW_ACCEL_SDPA_DEMO_K_ADDR 0x00005200u
#define HW_ACCEL_SDPA_DEMO_V_ADDR 0x00005300u
#define HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR 0x00005400u

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

/* tests/test_accels.c */
#include "accel_layout.h"
#include <stdint.h>

#define CUST_GEMM(rd, rs1, rs2)                                                \
  asm volatile(".insn r 0x0B, %3, 0, %0, %1, %2"                               \
               : "=r"(rd)                                                      \
               : "r"(rs1), "r"(rs2), "i"(HW_ACCEL_CUSTOM_FUNCT3_GEMM))

#define CUST_REDUCTION(rd, rs1, rs2)                                           \
  asm volatile(".insn r 0x0B, %3, 0, %0, %1, %2"                               \
               : "=r"(rd)                                                      \
               : "r"(rs1), "r"(rs2), "i"(HW_ACCEL_CUSTOM_FUNCT3_REDUCTION))

#define CUST_SDPA(rd, rs1, rs2)                                                \
  asm volatile(".insn r 0x0B, %3, 0, %0, %1, %2"                               \
               : "=r"(rd)                                                      \
               : "r"(rs1), "r"(rs2), "i"(HW_ACCEL_CUSTOM_FUNCT3_SDPA))

int main() {
  uint32_t gemm_a_ptr = HW_ACCEL_GEMM_DEMO_A_ADDR;
  uint32_t gemm_desc_ptr = HW_ACCEL_GEMM_DEMO_DESC_ADDR;
  uint32_t reduction_input_ptr = HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR;
  uint32_t reduction_desc_ptr = HW_ACCEL_REDUCTION_DEMO_DESC_ADDR;
  uint32_t sdpa_q_ptr = HW_ACCEL_SDPA_DEMO_Q_ADDR;
  uint32_t sdpa_desc_ptr = HW_ACCEL_SDPA_DEMO_DESC_ADDR;
  uint32_t status_gemm = 0;
  uint32_t status_reduction = 0;
  uint32_t status_sdpa = 0;

  CUST_GEMM(status_gemm, gemm_a_ptr, gemm_desc_ptr);
  CUST_REDUCTION(status_reduction, reduction_input_ptr, reduction_desc_ptr);
  CUST_SDPA(status_sdpa, sdpa_q_ptr, sdpa_desc_ptr);

  (void)status_gemm;
  (void)status_reduction;
  (void)status_sdpa;
  return 0;
}

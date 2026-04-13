#ifndef HW_ACCEL_H
#define HW_ACCEL_H

#include <stdint.h>

typedef enum {
  HW_ACCEL_STATUS_OK = 1u,
  HW_ACCEL_STATUS_ERR_ZERO_LENGTH = 2u,
  HW_ACCEL_STATUS_ERR_ZERO_DIMENSION = 3u,
  HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW = 4u,
  HW_ACCEL_STATUS_ERR_ADDRESS_RANGE = 5u,
  HW_ACCEL_STATUS_ERR_ALLOCATION = 6u,
} HwAccelStatus;

int hw_accel_init_demo_data(void);
uint32_t hw_accel_gemm(uint32_t matrix_a_addr, uint32_t desc_addr);
uint32_t hw_accel_reduction(uint32_t input_addr, uint32_t desc_addr);
uint32_t hw_accel_sdpa(uint32_t q_addr, uint32_t desc_addr);
const char *hw_accel_status_name(uint32_t status);

#endif // HW_ACCEL_H

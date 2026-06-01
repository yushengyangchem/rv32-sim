#include "accel_layout.h"
#include <stdint.h>

#define VOLATILE_STORE(addr, val) (*(volatile uint32_t *)(addr) = (val))

static void doorbell(uint32_t op, uint32_t desc_addr) {
  uint32_t val = (op << 24u) | (desc_addr & 0x00FFFFFFu);
  VOLATILE_STORE(HW_ACCEL_MMIO_DOORBELL, val);
}

int main() {
  doorbell(HW_ACCEL_OP_GEMM, HW_ACCEL_GEMM_DEMO_DESC_ADDR);
  doorbell(HW_ACCEL_OP_REDUCTION, HW_ACCEL_REDUCTION_DEMO_DESC_ADDR);
  doorbell(HW_ACCEL_OP_SDPA, HW_ACCEL_SDPA_DEMO_DESC_ADDR);
  return 0;
}

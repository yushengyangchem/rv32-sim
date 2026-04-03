#include "cpu_core.h"
#include "hw_accel.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  printf("========================================\n");
  printf(" RV32I Software Simulator + AI Kernels  \n");
  printf("========================================\n");

  if (argc < 2) {
    printf("Usage: %s <riscv_binary.bin>\n", argv[0]);
    return 1;
  }

  const char *bin_file = argv[1];

  // We assume the binary is linked to start at memory address 0
  uint32_t entry_point = 0x00000000;

  if (!mem_load_bin(bin_file, entry_point)) {
    return 1;
  }

  hw_accel_init_demo_data();

  CPU_State cpu;
  cpu_init(&cpu, entry_point);

  printf("\nStarting simulation...\n");
  cpu_run(&cpu);
  printf("Simulation finished.\n");

  return 0;
}

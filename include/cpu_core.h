#ifndef CPU_CORE_H
#define CPU_CORE_H

#include <stdbool.h>
#include <stdint.h>

// CPU State structure
typedef struct {
  uint32_t pc;       // Program Counter
  uint32_t regs[32]; // General Purpose Registers (x0 - x31)
  bool halted;       // Flag to stop the simulation
} CPU_State;

void cpu_init(CPU_State *cpu, uint32_t start_addr);
int cpu_step(CPU_State *cpu);
int cpu_run(CPU_State *cpu);

#endif // CPU_CORE_H

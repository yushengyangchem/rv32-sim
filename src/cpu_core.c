#include "cpu_core.h"
#include "accel_layout.h"
#include "hw_accel.h"
#include "memory.h"
#include <stdio.h>

void cpu_init(CPU_State *cpu, uint32_t start_addr) {
  for (int i = 0; i < 32; i++) {
    cpu->regs[i] = 0;
  }
  cpu->pc = start_addr;
  cpu->halted = false;
  printf("[CPU] Initialized. PC set to 0x%08X\n", cpu->pc);
}

int cpu_step(CPU_State *cpu) {
  if (cpu->halted)
    return 0;

  // 1. FETCH
  uint32_t inst = 0;
  if (mem_read_32(cpu->pc, &inst) != 0) {
    printf("[CPU] Memory read error at PC 0x%08X\n", cpu->pc);
    cpu->halted = true;
    return -1;
  }
  if (inst == 0) {
    printf("[CPU] Execution halted (fetched 0x00000000).\n");
    cpu->halted = true;
    return 0;
  }

  // 2. DECODE
  uint32_t opcode = inst & 0x7F;
  uint32_t rd = (inst >> 7) & 0x1F;
  uint32_t funct3 = (inst >> 12) & 0x7;
  uint32_t rs1 = (inst >> 15) & 0x1F;
  uint32_t rs2 = (inst >> 20) & 0x1F;
  uint32_t funct7 = (inst >> 25) & 0x7F;

  cpu->regs[0] = 0; // RISC-V spec dictates x0 is always 0

  // Default next instruction address is PC + 4
  uint32_t next_pc = cpu->pc + 4;

  // 3. EXECUTE
  switch (opcode) {
  case 0x03: { // --- I-Type (Load: LW, LH, LB) ---
    uint32_t imm12 = (inst >> 20) & 0xFFF;
    // Sign-extension for the load offset
    int32_t offset = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;

    uint32_t target_addr = cpu->regs[rs1] + offset;

    if (funct3 == 2) { // LW (Load Word - 32 bits)
      uint32_t val = 0;
      if (mem_read_32(target_addr, &val) != 0) {
        printf("[CPU] PC: 0x%08X | LW memory read error at 0x%08X\n", cpu->pc,
               target_addr);
        cpu->halted = true;
        cpu->pc = next_pc;
        return -1;
      }
      if (rd != 0)
        cpu->regs[rd] = val;
      printf("[CPU] PC: 0x%08X | LW x%d from Mem[0x%08X]\n", cpu->pc, rd,
             target_addr);
    } else {
      printf("[CPU] PC: 0x%08X | Ignore Load funct3: %d\n", cpu->pc, funct3);
    }
    break;
  }

  case 0x0B: { // --- CUSTOM-0 (Behavioral accelerator hooks) ---
    uint32_t status = 0;

    if (funct7 != 0) {
      printf("[CPU] PC: 0x%08X | Trap: Illegal Custom Instruction\n", cpu->pc);
      cpu->halted = true;
      cpu->pc = next_pc;
      return -1;
    }

    printf("[CPU] PC: 0x%08X | Decoding CUSTOM-0 Instruction...\n", cpu->pc);
    if (funct3 == HW_ACCEL_CUSTOM_FUNCT3_GEMM) {
      status = hw_accel_gemm(cpu->regs[rs1], cpu->regs[rs2]);
    } else if (funct3 == HW_ACCEL_CUSTOM_FUNCT3_REDUCTION) {
      status = hw_accel_reduction(cpu->regs[rs1], cpu->regs[rs2]);
    } else if (funct3 == HW_ACCEL_CUSTOM_FUNCT3_SDPA) {
      status = hw_accel_sdpa(cpu->regs[rs1], cpu->regs[rs2]);
    } else {
      printf("[CPU] PC: 0x%08X | Trap: Unknown Custom funct3 %u\n", cpu->pc,
             funct3);
      cpu->halted = true;
      cpu->pc = next_pc;
      return -1;
    }

    printf("[CPU] PC: 0x%08X | CUSTOM-0 status = %u (%s)\n", cpu->pc, status,
           hw_accel_status_name(status));

    if (rd != 0)
      cpu->regs[rd] = status;
    break;
  }

  case 0x13: { // --- I-Type (ADDI, etc.) ---
    uint32_t imm12 = (inst >> 20) & 0xFFF;
    // Sign-extension: If the 11th bit is 1, pad the upper bits with 1s
    int32_t imm = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;

    if (funct3 == 0) { // ADDI
      if (rd != 0)
        cpu->regs[rd] = cpu->regs[rs1] + imm;
      printf("[CPU] PC: 0x%08X | ADDI x%d, x%d, %d\n", cpu->pc, rd, rs1, imm);
    } else {
      printf("[CPU] PC: 0x%08X | Ignore I-Type funct3: %d\n", cpu->pc, funct3);
    }
    break;
  }

  case 0x23: { // --- S-Type (Store: SW, SH, SB) ---
    uint32_t imm11_5 = (inst >> 25) & 0x7F;
    uint32_t imm4_0 = (inst >> 7) & 0x1F;
    uint32_t imm_s = (imm11_5 << 5) | imm4_0;
    // Sign-extension for the store offset
    int32_t offset = (imm_s & 0x800) ? (imm_s | 0xFFFFF000) : imm_s;

    uint32_t target_addr = cpu->regs[rs1] + offset;

    if (funct3 == 2) { // SW (Store Word - 32 bits)
      if (mem_write_32(target_addr, cpu->regs[rs2]) != 0) {
        printf("[CPU] PC: 0x%08X | SW memory write error at 0x%08X\n", cpu->pc,
               target_addr);
        cpu->halted = true;
        cpu->pc = next_pc;
        return -1;
      }
      printf("[CPU] PC: 0x%08X | SW x%d into Mem[0x%08X]\n", cpu->pc, rs2,
             target_addr);
    } else {
      printf("[CPU] PC: 0x%08X | Ignore S-Type funct3: %d\n", cpu->pc, funct3);
    }
    break;
  }

  case 0x37: {                          // --- LUI (Load Upper Immediate) ---
    uint32_t u_imm = inst & 0xFFFFF000; // Extract the upper 20 bits
    if (rd != 0)
      cpu->regs[rd] = u_imm;
    printf("[CPU] PC: 0x%08X | LUI x%d, 0x%08X\n", cpu->pc, rd, u_imm);
    break;
  }

  case 0x67: { // --- JALR (Jump and Link Register, used for function returns)
               // ---
    uint32_t imm12 = (inst >> 20) & 0xFFF;
    int32_t imm = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;

    uint32_t target = (cpu->regs[rs1] + imm) & ~1; // Force the lowest bit to 0
    if (rd != 0)
      cpu->regs[rd] = cpu->pc + 4;
    next_pc = target; // Jump!
    printf("[CPU] PC: 0x%08X | JALR to 0x%08X\n", cpu->pc, next_pc);
    break;
  }

  case 0x6F: { // --- JAL (Jump and Link, used for function calls) ---
    // Intricate RISC-V J-Type immediate reconstruction logic
    uint32_t imm20 = (inst >> 31) & 0x1;
    uint32_t imm10_1 = (inst >> 21) & 0x3FF;
    uint32_t imm11 = (inst >> 20) & 0x1;
    uint32_t imm19_12 = (inst >> 12) & 0xFF;

    int32_t offset =
        (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    if (imm20)
      offset |= 0xFFE00000; // Sign-extension

    if (rd != 0)
      cpu->regs[rd] = cpu->pc + 4; // Save return address to rd (usually ra)
    next_pc = cpu->pc + offset;    // Jump!
    printf("[CPU] PC: 0x%08X | JAL to 0x%08X\n", cpu->pc, next_pc);
    break;
  }

  default:
    printf("[CPU] PC: 0x%08X | Trap: Unknown Opcode 0x%02X\n", cpu->pc, opcode);
    cpu->halted = true;
    cpu->pc = next_pc;
    return -1;
  }

  cpu->pc = next_pc;
  return 0;
}

int cpu_run(CPU_State *cpu) {
  while (!cpu->halted) {
    int rc = cpu_step(cpu);
    if (rc != 0)
      return rc;
  }
  return 0;
}

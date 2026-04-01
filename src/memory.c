#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The actual physical memory array for our simulator
static uint8_t ram[MEMORY_SIZE];

void mem_reset(void) { memset(ram, 0, sizeof(ram)); }

bool mem_load_bin(const char *filename, uint32_t base_addr) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    printf("[MEM] Error: Could not open file %s\n", filename);
    return false;
  }

  if (base_addr >= MEMORY_SIZE) {
    printf("[MEM] Error: Base address out of bounds\n");
    fclose(file);
    return false;
  }

  size_t bytes_read = fread(&ram[base_addr], 1, MEMORY_SIZE - base_addr, file);
  printf("[MEM] Loaded %zu bytes from %s starting at 0x%08X\n", bytes_read,
         filename, base_addr);

  fclose(file);
  return true;
}

uint32_t mem_read_32(uint32_t addr) {
  if (addr + 3 >= MEMORY_SIZE) {
    printf("[MEM] Warning: Out of bounds read at 0x%08X\n", addr);
    return 0;
  }
  // Little-endian read
  return (uint32_t)ram[addr] | ((uint32_t)ram[addr + 1] << 8) |
         ((uint32_t)ram[addr + 2] << 16) | ((uint32_t)ram[addr + 3] << 24);
}

void mem_write_32(uint32_t addr, uint32_t value) {
  if (addr + 3 >= MEMORY_SIZE) {
    printf("[MEM] Warning: Out of bounds write at 0x%08X\n", addr);
    return;
  }
  // Little-endian write
  ram[addr] = value & 0xFF;
  ram[addr + 1] = (value >> 8) & 0xFF;
  ram[addr + 2] = (value >> 16) & 0xFF;
  ram[addr + 3] = (value >> 24) & 0xFF;
}

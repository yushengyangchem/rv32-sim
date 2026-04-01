#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE (1024 * 1024) // 1MB of RAM for our simulator

void mem_reset(void);
bool mem_load_bin(const char *filename, uint32_t base_addr);
uint32_t mem_read_32(uint32_t addr);
void mem_write_32(uint32_t addr, uint32_t value);

#endif // MEMORY_H

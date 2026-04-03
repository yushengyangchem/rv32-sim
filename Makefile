# ==========================================
# rv32-sim Makefile
# ==========================================

# --- Host Simulator Configuration ---
CC_HOST = gcc
CFLAGS_HOST = -Wall -Wextra -Iinclude -O2
LDFLAGS_HOST = -lm
SRC_HOST = src/main.c src/cpu_core.c src/memory.c src/hw_accel.c src/kernels.c
BIN_HOST = rv32-sim
SRC_KERNEL_DEMO = src/kernel_demo.c src/kernels.c
BIN_KERNEL_DEMO = kernel-demo
SRC_KERNEL_TESTS = tests/kernel_tests.c src/kernels.c
BIN_KERNEL_TESTS = kernel-tests
SRC_HOST_TESTS = tests/host_tests.c src/memory.c src/hw_accel.c src/kernels.c
BIN_HOST_TESTS = host-tests
RTL_SIM = simv-tb
RTL_TB_SRCS = rtl/include/accel_layout_pkg.sv rtl/tb/simple_ram.sv rtl/examples/fake_gemm_dut.sv rtl/tb/accel_tb.sv
IVERILOG = iverilog
IVERILOG_FLAGS = -g2012
VVP = vvp

# --- Target RISC-V Configuration ---
CC_TARGET = riscv32-none-elf-gcc
OBJCOPY_TARGET = riscv32-none-elf-objcopy
# -march=rv32i: Base integer instruction set
# -mabi=ilp32: 32-bit int, long, pointers
# -nostdlib -ffreestanding: Bare-metal, no standard C library
CFLAGS_TARGET = -march=rv32i -mabi=ilp32 -nostdlib -ffreestanding -T tests/link.ld -O1 -Iinclude
SRC_TARGET = tests/start.S tests/test_accels.c
ELF_TARGET = tests/test_accels.elf
BIN_TARGET = tests/test_accels.bin

.PHONY: all clean run run-kernels test-kernels test gen-tb-init sim-tb

all: $(BIN_HOST) $(BIN_KERNEL_DEMO) $(BIN_KERNEL_TESTS) $(BIN_HOST_TESTS) $(BIN_TARGET)

# Build the host simulator
$(BIN_HOST): $(SRC_HOST)
	$(CC_HOST) $(CFLAGS_HOST) $^ -o $@ $(LDFLAGS_HOST)
	@echo "[BUILD] Host simulator built: $@"

$(BIN_KERNEL_DEMO): $(SRC_KERNEL_DEMO)
	$(CC_HOST) $(CFLAGS_HOST) $^ -o $@ $(LDFLAGS_HOST)
	@echo "[BUILD] Kernel demo built: $@"

$(BIN_KERNEL_TESTS): $(SRC_KERNEL_TESTS)
	$(CC_HOST) $(CFLAGS_HOST) $^ -o $@ $(LDFLAGS_HOST)
	@echo "[BUILD] Kernel tests built: $@"

$(BIN_HOST_TESTS): $(SRC_HOST_TESTS)
	$(CC_HOST) $(CFLAGS_HOST) $^ -o $@ $(LDFLAGS_HOST)
	@echo "[BUILD] Host accelerator tests built: $@"

# Build the target RISC-V ELF
$(ELF_TARGET): $(SRC_TARGET)
	$(CC_TARGET) $(CFLAGS_TARGET) $^ -o $@
	@echo "[BUILD] Target ELF built: $@"

# Convert the ELF to a raw binary for the simulator memory
$(BIN_TARGET): $(ELF_TARGET)
	$(OBJCOPY_TARGET) -O binary $< $@
	@echo "[BUILD] Target raw binary built: $@"

clean:
	rm -f $(BIN_HOST) $(BIN_KERNEL_DEMO) $(BIN_KERNEL_TESTS) $(BIN_HOST_TESTS) $(ELF_TARGET) $(BIN_TARGET) $(RTL_SIM)
	@echo "[CLEAN] Removed build artifacts."

# Convenience target to build and run immediately
run: all
	./$(BIN_HOST) $(BIN_TARGET)

run-kernels: $(BIN_KERNEL_DEMO)
	./$(BIN_KERNEL_DEMO)

test-kernels: $(BIN_KERNEL_TESTS)
	./$(BIN_KERNEL_TESTS)

test: $(BIN_HOST_TESTS)
	./$(BIN_HOST_TESTS)

gen-tb-init:
	bash rtl/tb/gen_demo_mem_init.sh

sim-tb: gen-tb-init
	$(IVERILOG) $(IVERILOG_FLAGS) -o $(RTL_SIM) $(RTL_TB_SRCS)
	$(VVP) $(RTL_SIM)

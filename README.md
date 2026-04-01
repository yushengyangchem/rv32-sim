# rv32-sim

A C-language learning project that combines:

- a small RV32I software simulator
- a custom instruction path for accelerator co-design
- software reference implementations of GeMM, SDPA, and Reduction
- a shared descriptor/memory-map contract for later RTL work

The goal is to learn both sides early:

- how a RISC-V CPU fetches, decodes, and executes instructions
- how core AI operators behave before moving to Verilog accelerators

## What This Project Covers

### 1. `rv32i-sim-c`

The simulator is written in C and runs a small bare-metal RISC-V program in software.

Current implemented instruction subset includes:

- `LUI`
- `ADDI`
- `LW`
- `SW`
- `JAL`
- `JALR`
- custom opcode `0x0B` for descriptor-driven accelerator experiments

This is intentionally a learning-oriented subset, not a full production RV32I implementation.

### 2. Core Operator Reference Models

This repository also includes C reference implementations for:

- `GeMM`
- `Reduction` (sum)
- `SDPA` (scaled dot-product attention)

These software kernels help validate:

- data layout
- loop structure
- accumulation flow
- attention score / softmax / weighted sum logic
- descriptor design before hardware implementation

That makes the project a good stepping stone toward later Verilog accelerator work.

## Current Status

The project is in a solid "learning-project but already structured" state:

- the RV32I simulator can fetch, decode, and execute a small bare-metal test binary
- `GeMM`, `Reduction`, and `SDPA` all have host-side C reference models
- all three accelerator entry points now use descriptor-driven metadata instead of hard-coded execution sizes
- host tests cover both the pure kernel outputs and the simulated accelerator memory-writeback flow
- RTL-facing address/descriptor definitions are mirrored in both C and SystemVerilog

## Build And Run

### Prerequisites

This project uses Nix to provide a reproducible toolchain.

```bash
nix develop
```

### Build everything

```bash
make all
```

### Run the RV32I simulator

```bash
make run
```

This runs the bare-metal RISC-V test program and exercises the `custom-0`
accelerator path for GeMM, Reduction, and SDPA.

### Run the software kernel demo

```bash
make run-kernels
```

This executes the C reference models for GeMM, Reduction, and SDPA directly on the host.

### Run the golden-reference tests

```bash
make test-kernels
```

This builds and runs host-side golden-reference checks for:

- GeMM reference output
- Reduction reference output
- SDPA reference output

```bash
make test
```

This builds and runs the host-side accelerator behavior test for:

- GeMM accelerator behavioral model output written back into simulated memory
- variable-shape GeMM descriptor execution
- Reduction accelerator output written back into simulated memory
- SDPA accelerator output written back into simulated memory

## Shared Interface

The accelerator memory map and descriptor structs are shared in [accel_layout.h](/home/yangys/repos/mine/github/rv32-sim/include/accel_layout.h).
The matching interface notes are documented in [accel_interface.md](/home/yangys/repos/mine/github/rv32-sim/docs/accel_interface.md).
For RTL and testbench work, the SystemVerilog mirror is [accel_layout_pkg.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/include/accel_layout_pkg.sv).
For memory preload in simulation, use [demo_mem_init.memh](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/demo_mem_init.memh) or regenerate it with `make gen-tb-init`.
A minimal testbench skeleton is available in [accel_tb.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/accel_tb.sv) with RAM in [simple_ram.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/simple_ram.sv).
There is also a runnable fake DUT example in [fake_gemm_dut.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/examples/fake_gemm_dut.sv); after `nix develop`, run `make sim-tb`.

## Suggested Roadmap

### Phase 1: Simulator Fundamentals

Goal: understand how a tiny ISA simulator works end to end.

- keep the current fetch / decode / execute loop easy to read
- add more RV32I instructions in small verified batches
- add focused instruction tests instead of growing one large demo program

### Phase 2: Software Kernels As Golden Models

Goal: understand operator math before touching RTL.

- keep `GeMM`, `Reduction`, and `SDPA` in C as golden references
- add more test shapes, edge cases, and failure-path validation
- document row-major layout, descriptor semantics, and expected outputs

### Phase 3: Accelerator Interface Contract

Goal: make the software/RTL boundary explicit and stable.

- keep all accelerator calls descriptor-driven
- share address maps and descriptor fields across C, docs, and SystemVerilog
- validate memory movement separately from compute implementation

### Phase 4: RTL Co-Design

Goal: replace behavioral models one operator at a time.

- start with GeMM writeback in RTL while keeping the C model as oracle
- extend the testbench from placeholder checks to descriptor-aware checking
- compare RTL results against the same memory layout and golden outputs

### Phase 5: Project Hardening

Goal: make the repo feel like a real long-lived learning platform.

- add CI for host builds/tests
- add negative tests for malformed descriptors
- add profiling, tracing, or simple performance counters in the simulator
- separate "teaching examples" from "regression tests" more clearly

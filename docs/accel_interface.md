# Accelerator Interface

This document defines the software-visible interface used by the simulator, host tests, and future RTL/testbench work.

## Custom Instruction Encoding

- Opcode: `0x0B` (`custom-0`)
- `funct7`: `0`
- `funct3 = 0`: GeMM
- `funct3 = 1`: Reduction
- `funct3 = 2`: SDPA

The current calling convention is:

- `rs1`: primary input base address
- `rs2`: descriptor address
- `rd`: status code written by the behavioral model

## Status Codes

The behavioral model writes one of these values to `rd`:

- `1`: `HW_ACCEL_STATUS_OK`
- `2`: `HW_ACCEL_STATUS_ERR_ZERO_LENGTH`
- `3`: `HW_ACCEL_STATUS_ERR_ZERO_DIMENSION`
- `4`: `HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW`
- `5`: `HW_ACCEL_STATUS_ERR_ADDRESS_RANGE`
- `6`: `HW_ACCEL_STATUS_ERR_ALLOCATION`

The C helper `hw_accel_status_name()` maps the numeric code to a readable
string for debug logs.
The same numeric constants are mirrored in
[accel_layout_pkg.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/include/accel_layout_pkg.sv)
so RTL and testbench code can check the same contract.

## Demo Memory Map

- `0x00001000`: GeMM matrix A
- `0x00002000`: GeMM matrix B
- `0x00003000`: GeMM result C
- `0x00003100`: GeMM descriptor
- `0x00004000`: Reduction input vector
- `0x00004100`: Reduction descriptor
- `0x00004200`: Reduction output scalar
- `0x00005000`: SDPA Q
- `0x00005100`: SDPA descriptor
- `0x00005200`: SDPA K
- `0x00005300`: SDPA V
- `0x00005400`: SDPA output

These constants live in [accel_layout.h](/home/yangys/repos/mine/github/rv32-sim/include/accel_layout.h).
The matching SystemVerilog package lives in [accel_layout_pkg.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/include/accel_layout_pkg.sv).

## Descriptor Formats

### Reduction Descriptor

Located at `rs2` for the Reduction instruction.

```c
typedef struct {
  uint32_t len;
  uint32_t output_addr;
} ReductionDescriptor;
```

Field meanings:

- `len`: number of `float32` input elements starting at `rs1`
- `output_addr`: address where the `float32` reduction result is written

### GeMM Descriptor

Located at `rs2` for the GeMM instruction.

```c
typedef struct {
  uint32_t b_addr;
  uint32_t output_addr;
  uint32_t rows;
  uint32_t cols;
  uint32_t depth;
} GemmDescriptor;
```

Field meanings:

- `b_addr`: base address of matrix B (`int32_t`)
- `output_addr`: base address of matrix C (`int32_t`)
- `rows`: output rows, also rows of matrix A
- `cols`: output cols, also cols of matrix B
- `depth`: reduction dimension shared by A and B

### SDPA Descriptor

Located at `rs2` for the SDPA instruction.

```c
typedef struct {
  uint32_t k_addr;
  uint32_t v_addr;
  uint32_t output_addr;
  uint32_t seq_len;
  uint32_t depth;
  uint32_t value_dim;
} SdpaDescriptor;
```

Field meanings:

- `k_addr`: base address of K matrix (`float32`)
- `v_addr`: base address of V matrix (`float32`)
- `output_addr`: base address of output matrix (`float32`)
- `seq_len`: sequence length
- `depth`: Q/K depth
- `value_dim`: V/output width

## Data Layout

- GeMM uses row-major `int32_t`, with A shaped `rows x depth`, B shaped `depth x cols`, and C shaped `rows x cols`
- Reduction uses contiguous `float32`
- SDPA uses row-major `float32` for `Q`, `K`, `V`, and output

## Current Demo Shapes

- GeMM: `2 x 2 x 2`
- Reduction length: `6`
- SDPA: `seq_len=2`, `depth=2`, `value_dim=2`

The demo memory image uses those values by default, but the host behavioral
models now execute using descriptor-provided sizes rather than hard-coded
dimensions.

## Why This Split Helps RTL

- C model, target program, and future RTL can share one source of truth for addresses
- Descriptor structs make the accelerator boundary explicit
- Testbench memory initialization can mirror the same layout without guessing

## Verilog Mapping

The SystemVerilog package provides:

- `localparam` definitions for `funct3` values
- `localparam` addresses for the demo memory map
- descriptor field byte offsets for bus-driven testbenches
- `gemm_descriptor_t`, `reduction_descriptor_t`, and `sdpa_descriptor_t` packed structs for RTL-side parsing

## Testbench Memory Init

A ready-to-use memory image is available at [demo_mem_init.memh](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/demo_mem_init.memh).
You can regenerate it with [gen_demo_mem_init.sh](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/gen_demo_mem_init.sh) or `make gen-tb-init`.

The file uses `readmemh` syntax with `@<word_addr>` jumps, so a word-addressed memory model can load it directly.

## Testbench Skeleton

A minimal SystemVerilog skeleton is available at:

- [simple_ram.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/simple_ram.sv)
- [accel_tb.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/tb/accel_tb.sv)

The skeleton currently provides:

- a simple 32-bit word-addressed RAM
- `$readmemh` preload from `demo_mem_init.memh`
- `accel_layout_pkg` imports
- placeholder checkpoints for GeMM, Reduction, and SDPA output regions

The repository now also includes a minimal fake DUT example at [fake_gemm_dut.sv](/home/yangys/repos/mine/github/rv32-sim/rtl/examples/fake_gemm_dut.sv).
It reads the GeMM descriptor, rejects simple malformed cases with shared status
codes, and for the valid demo case writes the expected result words into the
output buffer and returns `HW_ACCEL_STATUS_OK`.

You can run the example with `make sim-tb` after entering the Nix shell.

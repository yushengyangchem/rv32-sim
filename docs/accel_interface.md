# Accelerator Interface

This document defines the software-visible interface used by the simulator,
host tests, and future RTL/testbench work.

## MMIO Doorbell Interface

The accelerator is dispatched via **memory-mapped I/O** rather than custom
instructions. The CPU prepares a self-contained descriptor in regular memory,
then writes a single word to the accelerator's doorbell register to trigger the
operation. This mirrors how real SoC accelerators (NPU, DMA engines, etc.) are
driven.

### MMIO Register Map

| Address      | Name     | Access | Description                                |
| ------------ | -------- | ------ | ------------------------------------------ |
| `0x00090000` | DOORBELL | WO     | Write triggers an operation (see encoding) |
| `0x00090004` | STATUS   | RW     | Last operation status code                 |

### Doorbell Value Encoding

The 32-bit doorbell value is split into two fields:

```
[31:24]  op          – operation code (see below)
[23:0]   desc_addr   – byte address of the descriptor in memory
```

| `op` | Operation |
| ---- | --------- |
| `0`  | GeMM      |
| `1`  | Reduction |
| `2`  | SDPA      |

Example: to trigger a GeMM with the descriptor at `0x00082100`:

```c
*(volatile uint32_t*)0x00090000 = (0u << 24) | 0x00082100u;
```

Or equivalently in RISC-V assembly:

```asm
li   t0, 0x00082100
li   t1, 0x00090000
sw   t0, 0(t1)           # doorbell kick
```

### Status Codes

After each operation the accelerator writes one of these values to the STATUS
register:

| Value | Name                                 |
| ----- | ------------------------------------ |
| `1`   | `HW_ACCEL_STATUS_OK`                 |
| `2`   | `HW_ACCEL_STATUS_ERR_ZERO_LENGTH`    |
| `3`   | `HW_ACCEL_STATUS_ERR_ZERO_DIMENSION` |
| `4`   | `HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW`  |
| `5`   | `HW_ACCEL_STATUS_ERR_ADDRESS_RANGE`  |
| `6`   | `HW_ACCEL_STATUS_ERR_ALLOCATION`     |

The C helper `hw_accel_status_name()` maps the numeric code to a readable
string for debug logs.

## Demo Memory Map

| Address      | Content                 |
| ------------ | ----------------------- |
| `0x00080000` | GeMM matrix A           |
| `0x00081000` | GeMM matrix B           |
| `0x00082000` | GeMM result C           |
| `0x00082100` | GeMM descriptor         |
| `0x00083000` | Reduction input vector  |
| `0x00083100` | Reduction descriptor    |
| `0x00083200` | Reduction output scalar |
| `0x00084000` | SDPA Q                  |
| `0x00084100` | SDPA descriptor         |
| `0x00084200` | SDPA K                  |
| `0x00084300` | SDPA V                  |
| `0x00084400` | SDPA output             |
| `0x00090000` | **MMIO doorbell**       |
| `0x00090004` | **MMIO status**         |

These constants live in [accel_layout.h](../include/accel_layout.h).

## Descriptor Formats

All descriptors are **self-contained**: every address the accelerator needs is
stored inside the descriptor. No data addresses are passed through registers.

### GeMM Descriptor

```c
typedef struct {
  uint32_t a_addr;       // offset 0x00
  uint32_t b_addr;       // offset 0x04
  uint32_t output_addr;  // offset 0x08
  uint32_t rows;         // offset 0x0C
  uint32_t cols;         // offset 0x10
  uint32_t depth;        // offset 0x14
} GemmDescriptor;        // total 0x18 bytes
```

Field meanings:

- `a_addr`: base address of matrix A (`int32_t`, shape `rows x depth`)
- `b_addr`: base address of matrix B (`int32_t`, shape `depth x cols`)
- `output_addr`: base address of matrix C (`int32_t`, shape `rows x cols`)
- `rows`: output rows, also rows of matrix A
- `cols`: output cols, also cols of matrix B
- `depth`: reduction dimension shared by A and B

### Reduction Descriptor

```c
typedef struct {
  uint32_t input_addr;   // offset 0x00
  uint32_t len;          // offset 0x04
  uint32_t output_addr;  // offset 0x08
} ReductionDescriptor;   // total 0x0C bytes
```

Field meanings:

- `input_addr`: base address of the input vector (`float32`)
- `len`: number of `float32` elements
- `output_addr`: address where the `float32` reduction result is written

### SDPA Descriptor

```c
typedef struct {
  uint32_t q_addr;       // offset 0x00
  uint32_t k_addr;       // offset 0x04
  uint32_t v_addr;       // offset 0x08
  uint32_t output_addr;  // offset 0x0C
  uint32_t seq_len;      // offset 0x10
  uint32_t depth;        // offset 0x14
  uint32_t value_dim;    // offset 0x18
} SdpaDescriptor;        // total 0x1C bytes
```

Field meanings:

- `q_addr`: base address of Q matrix (`float32`, shape `seq_len x depth`)
- `k_addr`: base address of K matrix (`float32`, shape `seq_len x depth`)
- `v_addr`: base address of V matrix (`float32`, shape `seq_len x value_dim`)
- `output_addr`: base address of output matrix (`float32`, shape `seq_len x value_dim`)
- `seq_len`: sequence length
- `depth`: Q/K depth
- `value_dim`: V/output width

## Data Layout

- GeMM uses row-major `int32_t`
- Reduction uses contiguous `float32`
- SDPA uses row-major `float32` for Q, K, V, and output

## Current Demo Shapes

- GeMM: `2 x 2 x 3`
- Reduction length: `6`
- SDPA: `seq_len=2`, `depth=2`, `value_dim=2`

## Why MMIO + Self-Contained Descriptors

- **No custom instructions needed**: the CPU uses standard `SW` to talk to the
  accelerator, which is how real SoC software drives hardware blocks
- **One doorbell write per operation**: the accelerator DMA-reads the descriptor
  and all data, computes, then DMA-writes results back
- **Self-contained descriptors**: no implicit register-passing convention;
  everything the accelerator needs is in memory, which maps naturally to both
  the C behavioral model and future RTL
- **Status polling**: software can `LW` the STATUS register to check completion,
  mimicking real interrupt-or-poll semantics

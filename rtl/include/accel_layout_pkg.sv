package accel_layout_pkg;

  localparam logic [2:0] HW_ACCEL_CUSTOM_FUNCT3_GEMM = 3'd0;
  localparam logic [2:0] HW_ACCEL_CUSTOM_FUNCT3_REDUCTION = 3'd1;
  localparam logic [2:0] HW_ACCEL_CUSTOM_FUNCT3_SDPA = 3'd2;

  localparam logic [31:0] HW_ACCEL_STATUS_OK = 32'd1;
  localparam logic [31:0] HW_ACCEL_STATUS_ERR_ZERO_LENGTH = 32'd2;
  localparam logic [31:0] HW_ACCEL_STATUS_ERR_ZERO_DIMENSION = 32'd3;
  localparam logic [31:0] HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW = 32'd4;
  localparam logic [31:0] HW_ACCEL_STATUS_ERR_ADDRESS_RANGE = 32'd5;
  localparam logic [31:0] HW_ACCEL_STATUS_ERR_ALLOCATION = 32'd6;

  localparam logic [31:0] HW_ACCEL_GEMM_DEMO_A_ADDR = 32'h0000_1000;
  localparam logic [31:0] HW_ACCEL_GEMM_DEMO_B_ADDR = 32'h0000_2000;
  localparam logic [31:0] HW_ACCEL_GEMM_DEMO_C_ADDR = 32'h0000_3000;
  localparam logic [31:0] HW_ACCEL_GEMM_DEMO_DESC_ADDR = 32'h0000_3100;

  localparam logic [31:0] HW_ACCEL_REDUCTION_DEMO_INPUT_ADDR = 32'h0000_4000;
  localparam logic [31:0] HW_ACCEL_REDUCTION_DEMO_DESC_ADDR = 32'h0000_4100;
  localparam logic [31:0] HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR = 32'h0000_4200;

  localparam logic [31:0] HW_ACCEL_SDPA_DEMO_Q_ADDR = 32'h0000_5000;
  localparam logic [31:0] HW_ACCEL_SDPA_DEMO_DESC_ADDR = 32'h0000_5100;
  localparam logic [31:0] HW_ACCEL_SDPA_DEMO_K_ADDR = 32'h0000_5200;
  localparam logic [31:0] HW_ACCEL_SDPA_DEMO_V_ADDR = 32'h0000_5300;
  localparam logic [31:0] HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR = 32'h0000_5400;

  localparam int unsigned GEMM_DESC_B_ADDR_OFFSET = 0;
  localparam int unsigned GEMM_DESC_OUTPUT_ADDR_OFFSET = 4;
  localparam int unsigned GEMM_DESC_ROWS_OFFSET = 8;
  localparam int unsigned GEMM_DESC_COLS_OFFSET = 12;
  localparam int unsigned GEMM_DESC_DEPTH_OFFSET = 16;
  localparam int unsigned GEMM_DESC_BYTES = 20;

  localparam int unsigned REDUCTION_DESC_LEN_OFFSET = 0;
  localparam int unsigned REDUCTION_DESC_OUTPUT_ADDR_OFFSET = 4;
  localparam int unsigned REDUCTION_DESC_BYTES = 8;

  localparam int unsigned SDPA_DESC_K_ADDR_OFFSET = 0;
  localparam int unsigned SDPA_DESC_V_ADDR_OFFSET = 4;
  localparam int unsigned SDPA_DESC_OUTPUT_ADDR_OFFSET = 8;
  localparam int unsigned SDPA_DESC_SEQ_LEN_OFFSET = 12;
  localparam int unsigned SDPA_DESC_DEPTH_OFFSET = 16;
  localparam int unsigned SDPA_DESC_VALUE_DIM_OFFSET = 20;
  localparam int unsigned SDPA_DESC_BYTES = 24;

  typedef struct packed {
    logic [31:0] b_addr;
    logic [31:0] output_addr;
    logic [31:0] rows;
    logic [31:0] cols;
    logic [31:0] depth;
  } gemm_descriptor_t;

  typedef struct packed {
    logic [31:0] len;
    logic [31:0] output_addr;
  } reduction_descriptor_t;

  typedef struct packed {
    logic [31:0] k_addr;
    logic [31:0] v_addr;
    logic [31:0] output_addr;
    logic [31:0] seq_len;
    logic [31:0] depth;
    logic [31:0] value_dim;
  } sdpa_descriptor_t;

endpackage

module fake_gemm_dut (
    input  logic        clk,
    input  logic        rst_n,
    input  logic        start,
    input  logic [31:0] mem_rdata,
    output logic        done,
    output logic [31:0] status,
    output logic        mem_we,
    output logic [31:0] mem_addr,
    output logic [31:0] mem_wdata
);
  import accel_layout_pkg::*;

  typedef enum logic [3:0] {
    IDLE,
    READ_B_ADDR,
    READ_OUTPUT_ADDR,
    READ_ROWS,
    READ_COLS,
    READ_DEPTH,
    CHECK_DESC,
    WRITE_0,
    WRITE_1,
    WRITE_2,
    WRITE_3,
    FINISH
  } state_t;

  state_t state;
  logic [31:0] desc_b_addr;
  logic [31:0] desc_output_addr;
  logic [31:0] desc_rows;
  logic [31:0] desc_cols;
  logic [31:0] desc_depth;

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      state <= IDLE;
      done <= 1'b0;
      status <= '0;
      mem_we <= 1'b0;
      mem_addr <= '0;
      mem_wdata <= '0;
    end else begin
      mem_we <= 1'b0;
      done   <= 1'b0;

      case (state)
        IDLE: begin
          status <= '0;
          if (start) begin
            mem_addr <= HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_B_ADDR_OFFSET;
            state <= READ_B_ADDR;
          end
        end

        READ_B_ADDR: begin
          desc_b_addr <= mem_rdata;
          mem_addr <= HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_OUTPUT_ADDR_OFFSET;
          state <= READ_OUTPUT_ADDR;
        end

        READ_OUTPUT_ADDR: begin
          desc_output_addr <= mem_rdata;
          mem_addr <= HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_ROWS_OFFSET;
          state <= READ_ROWS;
        end

        READ_ROWS: begin
          desc_rows <= mem_rdata;
          mem_addr <= HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_COLS_OFFSET;
          state <= READ_COLS;
        end

        READ_COLS: begin
          desc_cols <= mem_rdata;
          mem_addr <= HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_DEPTH_OFFSET;
          state <= READ_DEPTH;
        end

        READ_DEPTH: begin
          desc_depth <= mem_rdata;
          state <= CHECK_DESC;
        end

        CHECK_DESC: begin
          if (desc_rows == 32'd0 || desc_cols == 32'd0 || mem_rdata == 32'd0) begin
            done   <= 1'b1;
            status <= HW_ACCEL_STATUS_ERR_ZERO_DIMENSION;
            state  <= IDLE;
          end else if (desc_b_addr != HW_ACCEL_GEMM_DEMO_B_ADDR ||
                       desc_output_addr != HW_ACCEL_GEMM_DEMO_C_ADDR) begin
            done   <= 1'b1;
            status <= HW_ACCEL_STATUS_ERR_ADDRESS_RANGE;
            state  <= IDLE;
          end else begin
            state <= WRITE_0;
          end
        end

        WRITE_0: begin
          mem_we <= 1'b1;
          mem_addr <= HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd0;
          mem_wdata <= 32'h0000003A;
          state <= WRITE_1;
        end

        WRITE_1: begin
          mem_we <= 1'b1;
          mem_addr <= HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd4;
          mem_wdata <= 32'h00000040;
          state <= WRITE_2;
        end

        WRITE_2: begin
          mem_we <= 1'b1;
          mem_addr <= HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd8;
          mem_wdata <= 32'h0000008B;
          state <= WRITE_3;
        end

        WRITE_3: begin
          mem_we <= 1'b1;
          mem_addr <= HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd12;
          mem_wdata <= 32'h0000009A;
          state <= FINISH;
        end

        FINISH: begin
          done   <= 1'b1;
          status <= HW_ACCEL_STATUS_OK;
          state  <= IDLE;
        end

        default: begin
          status <= '0;
          state  <= IDLE;
        end
      endcase
    end
  end

endmodule

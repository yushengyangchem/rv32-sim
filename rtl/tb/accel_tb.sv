`timescale 1ns / 1ps

module accel_tb;
  import accel_layout_pkg::*;

  // Match the 1 MiB host memory space so the shared demo addresses fit in RTL.
  localparam int unsigned RAM_WORDS = 262144;

  logic clk = 1'b0;
  logic rst_n = 1'b0;
  logic start = 1'b0;
  logic done;
  logic [31:0] status;
  logic we;
  logic [31:0] addr;
  logic [31:0] wdata;
  logic [31:0] rdata;

  fake_gemm_dut dut_i (
      .clk(clk),
      .rst_n(rst_n),
      .start(start),
      .mem_rdata(rdata),
      .done(done),
      .status(status),
      .mem_we(we),
      .mem_addr(addr),
      .mem_wdata(wdata)
  );

  simple_ram #(
      .WORDS(RAM_WORDS)
  ) ram_i (
      .clk(clk),
      .we(we),
      .addr(addr),
      .wdata(wdata),
      .rdata(rdata)
  );

  always #5 clk = ~clk;

  function automatic logic [31:0] read_word(input logic [31:0] target_addr);
    logic [31:0] word_addr;
    word_addr = target_addr >> 2;
    return ram_i.mem[word_addr];
  endfunction

  task automatic check_word(input string label, input logic [31:0] target_addr,
                            input logic [31:0] expected);
    logic [31:0] actual;
    actual = read_word(target_addr);
    if (actual !== expected) begin
      $error("%s mismatch at 0x%08h: got 0x%08h expected 0x%08h", label, target_addr, actual,
             expected);
    end else begin
      $display("[TB PASS] %s at 0x%08h = 0x%08h", label, target_addr, actual);
    end
  endtask

  task automatic preload_demo_mem;
    $readmemh("rtl/tb/demo_mem_init.memh", ram_i.mem);
    $display("[TB] Demo memory image loaded.");
  endtask

  task automatic write_word(input logic [31:0] target_addr, input logic [31:0] value);
    logic [31:0] word_addr;
    word_addr = target_addr >> 2;
    ram_i.mem[word_addr] = value;
  endtask

  task automatic check_status_constants;
    if (HW_ACCEL_STATUS_OK !== 32'd1 ||
          HW_ACCEL_STATUS_ERR_ZERO_LENGTH !== 32'd2 ||
          HW_ACCEL_STATUS_ERR_ZERO_DIMENSION !== 32'd3 ||
          HW_ACCEL_STATUS_ERR_SIZE_OVERFLOW !== 32'd4 ||
          HW_ACCEL_STATUS_ERR_ADDRESS_RANGE !== 32'd5 ||
          HW_ACCEL_STATUS_ERR_ALLOCATION !== 32'd6) begin
      $error("[TB] Accelerator status constants do not match C contract.");
    end else begin
      $display("[TB PASS] Accelerator status constants match C contract.");
    end
  endtask

  task automatic check_gemm_region;
    check_word("GeMM C[0][0]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd0, 32'h00000000);
    check_word("GeMM C[0][1]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd4, 32'h00000000);
    check_word("GeMM C[1][0]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd8, 32'h00000000);
    check_word("GeMM C[1][1]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd12, 32'h00000000);
  endtask

  task automatic check_reduction_region;
    check_word("Reduction output", HW_ACCEL_REDUCTION_DEMO_OUTPUT_ADDR, 32'h00000000);
  endtask

  task automatic check_sdpa_region;
    check_word("SDPA out[0][0]", HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + 32'd0, 32'h00000000);
    check_word("SDPA out[0][1]", HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + 32'd4, 32'h00000000);
    check_word("SDPA out[1][0]", HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + 32'd8, 32'h00000000);
    check_word("SDPA out[1][1]", HW_ACCEL_SDPA_DEMO_OUTPUT_ADDR + 32'd12, 32'h00000000);
  endtask

  task automatic run_placeholder_checks;
    $display("[TB] Checking preloaded output buffers before DUT writes.");
    check_gemm_region();
    check_reduction_region();
    check_sdpa_region();
  endtask

  task automatic trigger_dut;
    @(negedge clk);
    start = 1'b1;
    @(negedge clk);
    start = 1'b0;
    wait (done === 1'b1);
  endtask

  task automatic check_success_case;
    $display("[TB] Checking fake GeMM DUT success path.");
    check_word("GeMM C[0][0]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd0, 32'h0000003A);
    check_word("GeMM C[0][1]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd4, 32'h00000040);
    check_word("GeMM C[1][0]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd8, 32'h0000008B);
    check_word("GeMM C[1][1]", HW_ACCEL_GEMM_DEMO_C_ADDR + 32'd12, 32'h0000009A);
    if (status !== HW_ACCEL_STATUS_OK) begin
      $error("[TB] Fake GeMM DUT status mismatch: got 0x%08h expected 0x%08h", status,
             HW_ACCEL_STATUS_OK);
    end else begin
      $display("[TB PASS] Fake GeMM DUT status = 0x%08h", status);
    end
  endtask

  task automatic check_error_case(input string label, input logic [31:0] expected_status);
    if (status !== expected_status) begin
      $error("[TB] %s status mismatch: got 0x%08h expected 0x%08h", label, status, expected_status);
    end else begin
      $display("[TB PASS] %s status = 0x%08h", label, status);
    end
    check_gemm_region();
  endtask

  task automatic run_post_dut_checks;
    $display("[TB] Reduction and SDPA checkpoints are still placeholders.");
    check_reduction_region();
    check_sdpa_region();
  endtask

  initial begin
    preload_demo_mem();
    check_status_constants();
    repeat (2) @(negedge clk);
    rst_n = 1'b1;

    run_placeholder_checks();
    trigger_dut();
    check_success_case();
    run_post_dut_checks();

    preload_demo_mem();
    write_word(HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_ROWS_OFFSET, 32'd0);
    run_placeholder_checks();
    trigger_dut();
    check_error_case("Zero-dimension descriptor", HW_ACCEL_STATUS_ERR_ZERO_DIMENSION);

    preload_demo_mem();
    write_word(HW_ACCEL_GEMM_DEMO_DESC_ADDR + GEMM_DESC_OUTPUT_ADDR_OFFSET, 32'h0008_3330);
    run_placeholder_checks();
    trigger_dut();
    check_error_case("Address-range descriptor", HW_ACCEL_STATUS_ERR_ADDRESS_RANGE);

    $display("[TB] Fake GeMM DUT example completed.");
    $finish;
  end

endmodule

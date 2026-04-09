module simple_ram #(
    parameter int unsigned WORDS = 65536
) (
    input  logic        clk,
    input  logic        we,
    input  logic [31:0] addr,
    input  logic [31:0] wdata,
    output logic [31:0] rdata
);

  logic [31:0] mem[0:WORDS-1];

  wire [31:0] word_addr = addr >> 2;

  always_ff @(posedge clk) begin
    if (we) begin
      mem[word_addr] <= wdata;
    end
  end

  assign rdata = mem[word_addr];

endmodule

module test(
    input sys_clk_t,
    input sys_clk_n,

    output reg LED,
    output Lock,
    output clk
);

wire pll_in;
reg [28:0] counter = 0;
parameter second = 150_000_000;

TLVDS_IBUF sys_clk(
    .O(pll_in),

    .I(sys_clk_t),
    .IB(sys_clk_n)
);

always @(posedge clk) begin
    if (counter < second - 1) begin
        counter <= counter + 26'd1;
    end else begin
        counter <= 0;
    end
end

always @(posedge clk) begin
    if (counter < second / 2) begin
        LED <= 0;
    end else begin
        LED <= 1;
    end
end

Gowin_rPLL PLL(
    .clkout(clk), //output clkout
    .lock(Lock), // pll 
    .clkin(pll_in) //input clkin
);
endmodule
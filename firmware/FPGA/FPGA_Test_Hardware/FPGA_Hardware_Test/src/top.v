module test(
    output reg LED
);

wire clk;
reg [25:0] counter = 0;
parameter second = 25_000_000;

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

Gowin_OSC osc_inst(
    .oscout(clk)
);

endmodule
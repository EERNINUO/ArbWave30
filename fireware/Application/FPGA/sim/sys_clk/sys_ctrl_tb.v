/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 [EERNINUO]
 *
 * [This file is part of ArbWave30.]
 *
 * ArbWave30 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * ...
 */
`timescale 1ns/1ps

module sys_ctrl_tb();

parameter SYS_CLK_FREQ = 150_000_000; // 系统时钟频率，单位Hz
parameter DCO_CLK_FREQ = 150_000_000; // DCO时钟频率，单位Hz

// input declaration of module sys_ctrl
reg clk_in_p;
wire clk_in_n;
reg dco_clk;
reg ext_rst;

// output declaration of module sys_ctrl
wire sys_clk;
wire sys_rst_n;
wire dac_data_rst_n;
wire pll_lock;

sys_ctrl u_sys_ctrl(
    .clk_in_p       	(clk_in_p        ),
    .clk_in_n       	(clk_in_n        ),
    .dco_clk        	(dco_clk         ),
    .ext_rst        	(ext_rst         ),

    .sys_clk        	(sys_clk         ),
    .sys_rst_n      	(sys_rst_n       ),
    .dac_data_rst_n 	(dac_data_rst_n  ),
    .pll_lock       	(pll_lock        )
);

always  #10 clk_in_p = ~clk_in_p;
always  #3.333 dco_clk = ~dco_clk;

initial begin
    clk_in_p = 0;
    dco_clk = 0;
    ext_rst = 0;

    #100;
    ext_rst = 1;
    wait (pll_lock == 1);

    #100;
    ext_rst = 0;
    #100;
    ext_rst = 1;

    # 1000;
    $stop;
end

assign clk_in_n = ~clk_in_p;

endmodule
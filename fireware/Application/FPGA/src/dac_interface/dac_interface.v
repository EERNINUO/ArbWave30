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

module dac_interface (
    input  wire        sys_clk,
	input  wire        data_clk,
    input  wire        sys_rst_n,

    input  wire [15:0] ch1_data_in,
    output wire [15:0] ch1_data_out
);

    // internal signals                                            

FIFO_HS_Top your_instance_name(
	.Data(ch1_data_in), //input [15:0] Data
	.WrClk(sys_clk), //input WrClk
	.RdClk(data_clk), //input RdClk
	.WrEn(sys_rst_n), //input WrEn
	.RdEn(sys_rst_n), //input RdEn
	.Almost_Empty(), //output Almost_Empty
	.Almost_Full(), //output Almost_Full
	.Q(ch1_data_out), //output [15:0] Q
	.Empty(), //output Empty
	.Full() //output Full
);



endmodule
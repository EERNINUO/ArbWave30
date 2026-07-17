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

module dac_interface #(
	parameter SYS_CLK_FREQ = 150_000_000
)(
	// 系统接口
    input  wire        sys_clk,
	input  wire        data_clk,
    input  wire        sys_rst_n,

	// DAC 数据接口
    input  wire [15:0] ch1_data_in,
    output wire [15:0] ch1_data_out,

	// DAC 控制接口
	output wire        cs,
	output wire        clk,
	output wire        mosi,
	input 			   miso
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


SPI_master #(
	.SYS_CLK_FREQ 	(150_000_000)
) u_SPI_master (
	.sys_clk   	(sys_clk    ),
	.sys_rst_n 	(sys_rst_n  ),
	.cs        	(cs         ),
	.clk       	(clk        ),
	.mosi      	(mosi       ),
	.miso      	(miso       ),
	.send_data 	(8'h00      ),
	.start     	(1'd0),
	.rece_data 	(),
	.busy      	()
);



endmodule
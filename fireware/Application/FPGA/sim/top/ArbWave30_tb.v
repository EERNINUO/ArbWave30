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

module ArbWave30_tb;

parameter SYS_CLK_FREQ = 150_000_000; // 系统时钟频率，单位Hz
parameter DCO_CLK_FREQ = 150_000_000; // DCO时钟频率，单位Hz

// output declaration of module ArbWave30
wire ctrl_spi_miso;
wire [15:0] ch1_data_out;
wire dac_ctrl_spi_cs;
wire dac_ctrl_spi_clk;
wire dac_ctrl_spi_mosi;

reg clk_in_p;
wire clk_in_n;
reg ext_rst;
reg ctrl_spi_cs;
reg ctrl_spi_clk;
reg ctrl_spi_mosi;
reg ctrl_gpio1;
reg ctrl_gpio2;
reg dco_clk;

ArbWave30 u_ArbWave30(
    .clk_in_p          	(clk_in_p           ),
    .clk_in_n          	(clk_in_n           ),
    .ext_rst           	(ext_rst            ),

    .ctrl_spi_cs       	(ctrl_spi_cs        ),
    .ctrl_spi_clk      	(ctrl_spi_clk       ),
    .ctrl_spi_mosi     	(ctrl_spi_mosi      ),
    .ctrl_spi_miso     	(ctrl_spi_miso      ),
    .ctrl_gpio1        	(ctrl_gpio1         ),
    .ctrl_gpio2        	(ctrl_gpio2         ),
    .dco               	(dco_clk            ),
    .ch1_data_out      	(ch1_data_out       ),
    .dac_ctrl_spi_cs   	(dac_ctrl_spi_cs    ),
    .dac_ctrl_spi_clk  	(dac_ctrl_spi_clk   ),
    .dac_ctrl_spi_mosi 	(dac_ctrl_spi_mosi  ),
    .dac_ctrl_spi_miso 	(dac_ctrl_spi_miso  )
);

always  #10 clk_in_p = ~clk_in_p;
assign clk_in_n = ~clk_in_p;
always  #3.333 dco_clk = ~dco_clk;

initial begin
    clk_in_p = 1'b0;
    ext_rst = 1'b0;
    ctrl_spi_cs = 1'b1;
    ctrl_spi_clk = 1'b0;
    ctrl_spi_mosi = 1'b0;
    ctrl_gpio1 = 1'b0;
    ctrl_gpio2 = 1'b0;
    dco_clk = 1'b0;
    #100;
    ext_rst = 1'b1;

    #100000;
    $stop();
end

endmodule;

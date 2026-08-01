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

`timescale 1ns / 1ps

module reg_block_tb();

reg sys_clk;
reg sys_rst_n;

initial sys_clk = 0;
always #(3.333) sys_clk = ~sys_clk; // 仿真150MHz

// output declaration of module register_block
reg [6:0] addr;
reg [15:0] data_in;
wire [15:0] data_out;
reg write_en;
wire address_error;
wire soft_reset;
wire clock_source;
reg pll_lock;
wire ch1_enable;
wire [5:0] ch1_waveform;
wire [47:0] ch1_freq_ctrl_word;
wire [15:0] ch1_phase_ctrl_word;
wire [15:0] ch1_ampl_ctrl_word;
wire [15:0] ch1_dc_offset_word;
wire [9:0] ch1_duty_ctrl_word;
wire ch2_enable;
wire [5:0] ch2_waveform;
wire [47:0] ch2_freq_ctrl_word;
wire [15:0] ch2_phase_ctrl_word;
wire [15:0] ch2_ampl_ctrl_word;
wire [15:0] ch2_dc_offset_word;
wire [9:0] ch2_duty_ctrl_word;
wire [7:0] dac_reg_addr;
wire [7:0] dac_reg_data;
wire dac_reg_write;

register_block u_register_block(
    .sys_clk             	(sys_clk              ),
    .sys_rst_n           	(sys_rst_n            ),
    .addr                	(addr                 ),
    .data_in             	(data_in              ),
    .data_out            	(data_out             ),
    .write_en            	(write_en             ),
    .address_error       	(address_error        ),
    .soft_reset          	(soft_reset           ),
    .clock_source        	(clock_source         ),
    .pll_lock            	(pll_lock             ),
    .ch1_enable          	(ch1_enable           ),
    .ch1_waveform        	(ch1_waveform         ),
    .ch1_freq_ctrl_word  	(ch1_freq_ctrl_word   ),
    .ch1_phase_ctrl_word 	(ch1_phase_ctrl_word  ),
    .ch1_ampl_ctrl_word  	(ch1_ampl_ctrl_word   ),
    .ch1_dc_offset_word  	(ch1_dc_offset_word   ),
    .ch1_duty_ctrl_word  	(ch1_duty_ctrl_word   ),
    .ch2_enable          	(ch2_enable           ),
    .ch2_waveform        	(ch2_waveform         ),
    .ch2_freq_ctrl_word  	(ch2_freq_ctrl_word   ),
    .ch2_phase_ctrl_word 	(ch2_phase_ctrl_word  ),
    .ch2_ampl_ctrl_word  	(ch2_ampl_ctrl_word   ),
    .ch2_dc_offset_word  	(ch2_dc_offset_word   ),
    .ch2_duty_ctrl_word  	(ch2_duty_ctrl_word   ),
    .dac_reg_addr        	(dac_reg_addr         ),
    .dac_reg_data        	(dac_reg_data         ),
    .dac_reg_write       	(dac_reg_write        )
);


task reg_write;
    input [6:0] address;
    input [15:0] input_data;
begin
    addr = address;
    data_in = input_data;
    #10
    write_en = 1;
    #50;
    write_en = 0;
end
endtask

task reg_read;
    input [6:0] address;
begin
    addr = address;
    #10;
end
endtask

initial begin
    addr = 0;
    data_in = 0;
    sys_rst_n = 0;
    write_en = 0;
    pll_lock = 0;
    #100;
    sys_rst_n = 1;
    #100;

    pll_lock = 0;

    reg_read(7'h00);
    $display("reg_read(7'h00) = %h", data_out);
    # 50;
    reg_read(7'h01);
    $display("reg_read(7'h01) = %h", data_out);

    reg_write(7'h11, 16'h9ABC); 
    # 50;
    reg_write(7'h12, 16'h5678); 
    # 50;
    reg_write(7'h13, 16'h1234);
    # 50;
    reg_write(7'h02, 16'h0008);
    # 100;
    
    reg_read(7'h05); 

    pll_lock = 1;

    # 50;
    reg_read(7'h03);
    $display("reg_read(7'h03) = %h", data_out);
    $stop;
end

endmodule
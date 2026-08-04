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
wire fpga_int;
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
    .fpga_int        	(fpga_int         ),
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

//============================
// SPI 任务函数
//============================

// CRC 计算函数
function automatic [7:0] crc;
    input [7:0] crcIn;
    input [23:0] data;
    integer i;
    reg [7:0] lfsr;
begin
    lfsr = crcIn;
    for (i = 23; i >= 0; i = i - 1) begin
        if (lfsr[7] ^ data[i]) begin
            lfsr = (lfsr << 1) ^ 8'h07;
        end else begin
            lfsr = (lfsr << 1);
        end
    end
    crc = lfsr;
end
endfunction

// SPI 发送一个字节（通过 MOSI 发送）
task send_byte;
    input [7:0] data;
    integer bit_idx;
begin
    for (bit_idx = 7; bit_idx >= 0; bit_idx = bit_idx - 1) begin
        ctrl_spi_clk = 0;
        # 50
        ctrl_spi_mosi = data[bit_idx];
        # 1;
        ctrl_spi_clk = 1;
        # 50;
        ctrl_spi_clk = 0;
    end
end
endtask

// 接收一个字节（从 MISO 读取）
task receive_byte;
    output [7:0] byte_data;
    integer i;
begin
    byte_data = 8'h00;
    for (i = 7; i >= 0; i = i - 1) begin
        ctrl_spi_clk = 0;
        # 50
        byte_data[i] = ctrl_spi_miso;
        # 1;
        ctrl_spi_clk = 1;
        # 50;
        ctrl_spi_clk = 0;
    end
end
endtask

// SPI 写操作任务
task spi_write;
    input [6:0] reg_addr;
    input [15:0] write_data;
    reg [7:0] ack_received;
begin

    ctrl_spi_cs = 1'b0;
    send_byte({1'b0, reg_addr});// 发送地址（写操作：bit7=0）

    send_byte(write_data[7:0]);
    send_byte(write_data[15:8]);

    send_byte(crc(8'h00, {1'b0, reg_addr, write_data[7:0], write_data[15:8]})); // 发送 CRC

    receive_byte(ack_received);// 接收 ACK

    # 2;
    ctrl_spi_cs = 1'b1;

    $display("SPI Write: Address = 0x%02h, Data = 0x%04h, ACK = 0x%02h", reg_addr, write_data, ack_received);
end
endtask

// SPI 读操作任务
task spi_read;
    input [6:0] reg_addr;
    output [15:0] receive_data;  // 期望读回的数据
    reg [7:0] crc_received;
    reg [7:0] ack_received;
    reg [7:0] data_l, data_h;
begin
    ctrl_spi_cs = 1'b0;
    
    send_byte({1'b1, reg_addr});// 发送地址（读操作：bit7=1）

    receive_byte(data_l);
    receive_byte(data_h);
    
    receive_byte(crc_received);  // 接收 CRC

    receive_byte(ack_received);  // 接收 ACK
    ctrl_spi_cs = 1'b1;

    receive_data = {data_h, data_l};

    $display("SPI Read: Address = 0x%02h, Data = 0x%04h, CRC = 0x%02h, ACK = 0x%02h", reg_addr, receive_data, crc_received, ack_received);
end
endtask

//============================
// 测试代码
//============================
localparam freq_ctrl_word_1MHz = 48'd1_876_499_844_737;
localparam freq_ctrl_word_10MHz = 48'd18_764_998_447_377;

initial begin
    clk_in_p = 1'b0;
    ext_rst = 1'b0;
    ctrl_spi_cs = 1'b1;
    ctrl_spi_clk = 1'b0;
    ctrl_spi_mosi = 1'b0;
    ctrl_gpio2 = 1'b0;
    dco_clk = 1'b0;
    #100;
    ext_rst = 1'b1;

    wait (fpga_int == 1'b1);

    // 频率控制字
    spi_write(7'h11, freq_ctrl_word_10MHz[15:0]); 
    # 4000;
    spi_write(7'h12, freq_ctrl_word_10MHz[31:16]); 
    # 4000;
    spi_write(7'h13, freq_ctrl_word_10MHz[47:32]); 
    # 4000;

    // 幅度控制字
    spi_write(7'h14, 16'h8fff); 
    # 4000;
    // 打开通道1
    spi_write(7'h10, 16'h8000);
    # 4000;

    // 影子寄存器更新
    spi_write(7'h02, 16'h0008);
    # 4000;

    // 频率控制字
    spi_write(7'h11, freq_ctrl_word_1MHz[15:0]); 
    # 4000;
    spi_write(7'h12, freq_ctrl_word_1MHz[31:16]); 
    # 4000;
    spi_write(7'h13, freq_ctrl_word_1MHz[47:32]); 
    # 4000;

    // 影子寄存器更新
    spi_write(7'h02, 16'h0008);
    # 8000;

    spi_write(7'h14, 16'h3FFF);
    # 4000;
    spi_write(7'h15, 16'h3FFF);
    # 4000;
    spi_write(7'h02, 16'h0008);
    # 4000;

    // 测试写入错误地址的情况
    spi_write(7'h1f, 16'h0000);

    # 10000;

    $stop();
end

endmodule;

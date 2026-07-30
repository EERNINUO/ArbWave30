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

module spi_slave_tb();

// output declaration of module spi_slave
reg sys_clk;
reg sys_rst_n;

reg spi_clk;
wire spi_miso;
reg spi_cs;
reg spi_mosi;

wire [6:0] addr;
wire [15:0] data_out;
reg [15:0] data_in;
wire out_data_valid;
reg address_error;

spi_slave #(
    .SYS_CLK_FREQ 	(150_000_000  ))
u_spi_slave(
    .sys_clk        	(sys_clk         ),
    .sys_rst_n      	(sys_rst_n       ),
    .spi_clk        	(spi_clk         ),
    .spi_cs         	(spi_cs          ),
    .spi_mosi       	(spi_mosi        ),
    .spi_miso       	(spi_miso        ),
    .addr           	(addr            ),
    .data_out       	(data_out        ),
    .data_in        	(data_in         ),
    .out_data_valid 	(out_data_valid  ),
    .address_error  	(address_error   )
);

// 时钟生成
// 系统时钟：150MHz (周期 ≈ 6.667ns)
initial sys_clk = 0;
always #(3.333) sys_clk = ~sys_clk; // 仿真150MHz

// SPI 时钟：故意设为非整数频率，模拟异步关系
initial spi_clk = 0;
always #(50.234) spi_clk = ~spi_clk; 

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
        @(negedge spi_clk) begin
            # 5
            spi_mosi = data[bit_idx];
        end
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
        @(posedge spi_clk) begin
            # 5 
            byte_data[i] = spi_miso;
        end
    end
end
endtask

// SPI 写操作任务
task spi_write;
    input [6:0] reg_addr;
    input [15:0] write_data;
    input [7:0] expected_ack;  // 期望的ACK值 (用于校验)
    reg [7:0] ack_received;
begin

    spi_cs = 1'b0;
    send_byte({1'b0, reg_addr});// 发送地址（写操作：bit7=0）

    send_byte(write_data[7:0]);
    send_byte(write_data[15:8]);

    send_byte(crc(8'h00, {1'b0, reg_addr, write_data[7:0], write_data[15:8]})); // 发送 CRC

    receive_byte(ack_received);// 接收 ACK
    @(posedge spi_clk);
    # 2;
    spi_cs = 1'b1;

    #100;
end
endtask

// SPI 读操作任务
task spi_read;
    input [6:0] reg_addr;
    input [15:0] expected_data;  // 期望读回的数据
    reg [7:0] crc_received;
    reg [7:0] ack_received;
    reg [15:0] read_data;
    reg [7:0] data_l, data_h;
begin
    @(posedge sys_clk);
    # 2;
    spi_cs = 1'b0;
    
    send_byte({1'b1, reg_addr});// 发送地址（读操作：bit7=1）

    receive_byte(data_l);
    receive_byte(data_h);
    
    receive_byte(crc_received);  // 接收 CRC

    receive_byte(ack_received);  // 接收 ACK
    @(posedge spi_clk);
    spi_cs = 1'b1;

    read_data = {data_h, data_l};

    // $display("Read from address 0x%02h: received data = 0x%04h, expected data = 0x%04h", reg_addr, read_data, expected_data);

    // if (crc_received !== crc(8'h00, {1'b1, reg_addr, data_l, data_h})) begin
    //     $display("rece CRC error: expected %h, received %h", crc(8'h00, {1'b1, reg_addr, data_l, data_h}), crc_received);
    // end
end
endtask

initial begin
    data_in = 16'h0000;
    spi_cs = 1'b1;
    address_error = 1'b0;

    sys_rst_n = 1'b0;
    #100;
    sys_rst_n = 1'b1;
end

initial begin
    wait (sys_rst_n == 1);

    // 测试写操作
    spi_write(7'h02, 16'hABCD, 8'h01); // 写入寄存器地址0x01，数据0xABCD
    #1000;

    data_in = 16'hABCD; // 设置期望读回的数据
    spi_read(7'h02, 16'hABCD); // 读取寄存器地址0x01，期望数据0xABCD
    #1000;

    $stop;
end

endmodule
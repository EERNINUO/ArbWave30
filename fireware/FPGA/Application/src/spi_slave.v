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

module spi_slave #(
    parameter SYS_CLK_FREQ = 150_000_000
)(
    input                sys_clk,
    input                sys_rst_n,

    input                spi_clk,
    input                spi_cs,
    input                spi_mosi,
    output  reg          spi_miso,

    output  reg  [6:0]   addr,
    output  reg  [15:0]  data_out,
    input        [15:0]  data_in,
    output  reg          out_data_valid,
    input                address_error
);

// SPI clk 亚稳态处理
reg spi_clk_d1;
reg spi_clk_d2;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_clk_d1 <= 1'b0;
        spi_clk_d2 <= 1'b0;
    end else begin
        spi_clk_d1 <= spi_clk;
        spi_clk_d2 <= spi_clk_d1;
    end
end

// SPI CS 亚稳态处理
reg spi_cs_d1;
reg spi_cs_d2;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_cs_d1 <= 1'b0;
        spi_cs_d2 <= 1'b0;
    end else begin
        spi_cs_d1 <= spi_cs;
        spi_cs_d2 <= spi_cs_d1;
    end
end

// SPI clk 边沿检测
reg spi_clk_dly;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_clk_dly <= 1'b0;
    end
    else begin
        spi_clk_dly <= spi_clk_d2;
    end
end

wire spi_clk_posedge = spi_clk_d2 & !spi_clk_dly;
wire spi_clk_negedge = !spi_clk_d2 & spi_clk_dly; 

// 将 SPI_CLK_posedge 延迟一个时钟周期，以便在 SPI 状态机中使用
reg spi_clk_posedge_dly;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_clk_posedge_dly <= 1'b0;
    end
    else begin
        spi_clk_posedge_dly <= spi_clk_posedge;
    end
end

// CRC 校验函数
reg [7:0] crc_reg;

function automatic [7:0] crc;
    input [7:0] crcIn;
    input [0:0] data;
begin
    crc[0] = crcIn[7] ^ data[0];
    crc[1] = crcIn[0] ^ crcIn[7] ^ data[0];
    crc[2] = crcIn[1] ^ crcIn[7] ^ data[0];
    crc[3] = crcIn[2];
    crc[4] = crcIn[3];
    crc[5] = crcIn[4];
    crc[6] = crcIn[5];
    crc[7] = crcIn[6];
end
endfunction

// SPI 超时检测
localparam TIMEOUT_US = 1000; // 超时时间，单位微秒
localparam TIMEOUT_CYCLES = (SYS_CLK_FREQ / 1_000_000) * TIMEOUT_US; // 超时计数器的最大值

reg [$clog2(TIMEOUT_CYCLES)-1:0] spi_timeout_cnt; // 超时计数器

always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_timeout_cnt <= 0;
    end
    else if (spi_cs_d2) begin
        spi_timeout_cnt <= 0;
    end
    else if (spi_timeout_cnt < TIMEOUT_CYCLES - 1) begin
        spi_timeout_cnt <= spi_timeout_cnt + 1'b1;
    end
end

//==============================================================
// SPI 状态机
//==============================================================
// SPI 状态定义
localparam  SPI_IDLE = 3'h0;
localparam  SPI_ADDR = 3'h1;
localparam  SPI_DATA = 3'h2;
localparam  SPI_CRC  = 3'h3;
localparam  SPI_ACK  = 3'h4;

reg [2:0] spi_state;

// SPI 读写标志位
localparam  WR_bit = 3'h7;

// ACK 信号定义
localparam  ACK_bit = 3'h0;
localparam  ADDR_ERR_bit = 3'h1;
localparam  BUSY = 3'h2;
localparam  CRC_ERR = 3'h3;

// SPI 数据寄存器
reg  [7:0]   spi_address;
reg  [15:0]  rx_data;
reg  [15:0]  tx_data;
reg  [7:0]   crc_received;
reg  [7:0]   ack;

reg [3:0] spi_cnt;

// 状态转移控制
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        spi_state <= SPI_IDLE;
        spi_cnt <= 4'h0;
    end
    else begin
        case(spi_state)
        SPI_IDLE: begin
            if (!spi_cs_d2) begin
                spi_state <= SPI_ADDR;
            end
        end
        SPI_ADDR: begin
            if (spi_cs_d2 || spi_timeout_cnt == (TIMEOUT_CYCLES - 1)) begin
                spi_state <= SPI_IDLE;
            end
            else if (spi_cnt == 4'h7 && spi_clk_posedge_dly) begin
                spi_cnt <= 4'h0;
                spi_state <= SPI_DATA;
            end
            else if (spi_clk_posedge_dly) begin
                spi_cnt <= spi_cnt + 1'b1;
            end
        end
        SPI_DATA: begin
            if (spi_cs_d2 || spi_timeout_cnt == (TIMEOUT_CYCLES - 1)) begin
                spi_state <= SPI_IDLE;
            end
            else if (spi_cnt == 4'hf && spi_clk_posedge_dly) begin
                spi_cnt <= 4'h0;
                spi_state <= SPI_CRC;
            end
            else if (spi_clk_posedge_dly) begin
                spi_cnt <= spi_cnt + 1'b1;
            end
        end
        SPI_CRC: begin
            if (spi_cs_d2 || spi_timeout_cnt == (TIMEOUT_CYCLES - 1)) begin
                spi_state <= SPI_IDLE;
            end
            else if (spi_cnt == 4'h7 && spi_clk_posedge_dly) begin
                spi_cnt <= 4'h0;
                spi_state <= SPI_ACK;
            end
            else if (spi_clk_posedge_dly) begin
                spi_cnt <= spi_cnt + 1'b1;
            end
        end
        SPI_ACK: begin
            if ((spi_cnt == 4'h7 && spi_clk_posedge_dly) || 
                spi_cs_d2 || 
                spi_timeout_cnt == (TIMEOUT_CYCLES - 1)) begin
                spi_cnt <= 4'h0;
                spi_state <= SPI_IDLE;
            end
            else if (spi_clk_posedge_dly) begin
                spi_cnt <= spi_cnt + 1'b1;
            end
        end
        default: spi_state <= SPI_IDLE;
        endcase
    end
end

// 状态机输出控制
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
    end 
    else begin
        case(spi_state)
        SPI_IDLE: begin
            spi_miso <= 1'bz;
            spi_address <= 8'h00;
            rx_data <= 16'h0000;
            tx_data <= 16'h0000;
            crc_reg <= 8'h00;
            crc_received <= 8'h00;
            ack <= 8'h00;

            data_out <= 16'h0000;
            out_data_valid <= 1'b0;
        end
        SPI_ADDR: begin
            if (spi_clk_posedge) begin
                spi_address <= {spi_address[6:0], spi_mosi};
                crc_reg <= crc(crc_reg, spi_mosi);
                spi_miso <= 1'b0;
            end
            if (spi_clk_posedge_dly && spi_cnt == 4'h7) begin
                addr <= spi_address[6:0];
                // 数据传输开始，准备发送数据
                if (spi_address[WR_bit] == 1'b1) begin
                    // 读操作，准备发送数据
                    tx_data <= {data_in[7:0], data_in[15:8]}; // 交换字节顺序
                end
            end
        end
        SPI_DATA: begin
            if (spi_cnt == 4'h0) begin
                if (address_error) begin
                    ack[ADDR_ERR_bit] <= 1'b1;
                end
            end
            if (spi_address[WR_bit] == 1'b0) begin
                // 写操作
                if (spi_clk_posedge) begin
                    rx_data <= {rx_data[14:0], spi_mosi};
                    crc_reg <= crc(crc_reg, spi_mosi);
                end
            end
            else begin
                // 读操作
                if (spi_clk_negedge) begin
                    spi_miso <= tx_data[15];
                    tx_data <= {tx_data[14:0], 1'b0};
                    crc_reg <= crc(crc_reg, tx_data[15]);
                end
            end
        end
        SPI_CRC: begin
            if (spi_address[WR_bit] == 1'b0) begin
                // 写操作
                if (spi_clk_posedge) begin
                    crc_received <= {crc_received[6:0], spi_mosi};
                end
                if (spi_cnt == 4'h7) begin
                    // 检查 CRC
                    if (crc_received == crc_reg)begin
                        ack[CRC_ERR] <= 1'b0;
                        data_out <= {rx_data[7:0], rx_data[15:8]}; // 交换字节顺序
                    end
                    else begin
                        ack[CRC_ERR] <= 1'b1;
                    end
                end 
            end
            else begin
                // 读操作
                if (spi_clk_negedge) begin
                    spi_miso <= crc_reg[7];
                    crc_reg <= {crc_reg[6:0], 1'b0};
                end
            end
        end
        SPI_ACK: begin
            if (spi_cnt == 4'h0) begin
                ack[ACK_bit] <= ~(|ack[7:1]); // 如果没有错误，ACK 位为 1，否则为 0
            end
            if (spi_clk_negedge) begin
                spi_miso <= ack[7];
                ack <= {ack[6:0], 1'b0};
            end
            if (spi_address[WR_bit] == 1'b0 && spi_cnt == 4'h7) begin
                // 写入数据有效
                out_data_valid <= 1'b1;
            end
        end
        endcase
    end
end


endmodule
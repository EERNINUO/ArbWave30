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

module SPI_master#(
    parameter SYS_CLK_FREQ = 150_000_000,
    parameter SPI_CLK_DIV = 10 // 分频系数，目前只能是偶数
)(
    // 系统时钟
    input               sys_clk,
    input               sys_rst_n,
    
    // SPI 接口
    output  reg         cs,
    output  reg         clk,
    output  reg         mosi,
    input               miso,
    
    // 数据接口
    input        [7:0]  send_data,
    input               start,
    output  reg  [7:0]  rece_data,
    output  reg         busy
);

// 状态机
localparam  IDLE = 3'b000,
            SEND = 3'b001,
            DONE = 3'b010;
reg [2:0] state;

// start 信号边沿检测
reg start_dly = 1'b0;
wire start_posedge;

always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        start_dly <= 1'b0;
    end
    else begin
        start_dly <= start;
    end
end

assign start_posedge = start & ~start_dly;

// SPI 时钟分频
localparam DIV_HALF = SPI_CLK_DIV / 2;
localparam DIV_CNT_LEN = $clog2(DIV_HALF);

reg [DIV_CNT_LEN-1 : 0] clk_div;

// SPI 发送数据
reg [3:0] bit_send_cnt;
reg [7:0] send_data_reg;

always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        cs <= 1'b1;
        clk <= 1'b0;
        mosi <= 1'b0;
        clk_div <= 'h0;
        rece_data <= 'h0;
    end
    else begin
        case (state) 
            IDLE: begin
                cs <= 1'b1;
                clk <= 1'b0;
                mosi <= 1'b0;
                busy <= 1'b0;
                clk_div <= 'h0;
                rece_data <= 'h0;
                if (start_posedge) begin
                    busy <= 1'b1;
                    send_data_reg <= send_data;
                    cs <= 1'b0;
                end
            end
            SEND: begin
                if (clk_div == DIV_HALF - 1) begin
                    clk_div <= 'h0;
                    clk <= ~clk;
                    if (clk) begin
                        mosi <= send_data_reg[7];
                        send_data_reg <= {send_data_reg[6:0], 1'b0};
                        rece_data <= {rece_data[6:0], miso};
                        bit_send_cnt <= bit_send_cnt + 1'b1;
                    end
                end
                else begin
                    clk_div <= clk_div + 1'b1;
                end
            end
            DONE: begin
                cs <= 1'b1;
                clk <= 1'b0;
                mosi <= 1'b0;
                clk_div <= 'h0;
                busy <= 1'b0;
            end
        endcase
    end
end

// 状态机控制
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        state <= IDLE;
    end
    else begin
        case (state) 
            IDLE: if (start_posedge) state <= SEND;
            SEND: if (bit_send_cnt == 4'd8) state <= DONE;
            DONE: state <= IDLE;
        endcase
            
    end
end

endmodule
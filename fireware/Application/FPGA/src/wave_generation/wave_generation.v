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

module wave_generation(
    input                sys_clk,              // 系统时钟
    input                sys_rst_n,            // 系统复位，低有效

    input                ch1_enable,           // 通道1使能信号
    input        [15: 0] ch1_amplitude,        // 通道1幅度控制，16位，范围0~65535
    input signed [15: 0] ch1_offset,           // 通道1偏置控制，
    input        [47: 0] ch1_freq_ctrl_word,   // 通道1频率控制字，48位，范围0~2^48-1
    input        [47: 0] ch1_phase_ctrl_word,  // 通道1相位控制字，48位，范围0~2^48-1

    output  reg signed [15: 0] ch1_data_out          // 通道1输出数据，16位，范围0~65535
);

wire                   ch1_rst_n;  // 通道1复位信号，低有效
wire  signed  [15: 0]  ch1_sine_out;  // 通道1正弦波输出，16位，范围-32768~32767

assign ch1_rst_n = sys_rst_n & ch1_enable;  // 通道1复位信号，低有效 

// 正弦波 DDS 实例化（高云 IP 核）
DDS_II_Top u_dds_ii_ch1 (
    .clk_i(sys_clk),                        // 输入时钟
    .rst_n_i(ch1_rst_n),                    // 输入复位，低有效
    .phase_valid_i(ch1_rst_n),              // 相位有效信号，始终为高电平
    .phase_inc_i(ch1_freq_ctrl_word),       // 频率控制字输入
    .phase_off_i(ch1_phase_ctrl_word),      // 相位控制字输入
    .phase_out_o(),                         // 相位输出
    .sine_o(ch1_sine_out),                  // 正弦波输出
    .data_valid_o()                         // 数据有效信号
);

always @(posedge sys_clk or negedge ch1_rst_n) begin
    if (!ch1_rst_n) begin
        ch1_data_out <= 16'd0;  // 复位时输出中间值（偏置为0）
    end else begin
        // 将正弦波输出进行幅度和偏置调整
        // ch1_data_out <= ch1_sine_out * ch1_amplitude) + ch1_offset;
        ch1_data_out <= ch1_sine_out;  
    end
end

endmodule
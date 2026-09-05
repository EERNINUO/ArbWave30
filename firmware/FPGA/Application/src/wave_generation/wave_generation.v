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
    input                    sys_clk,              // 系统时钟
    input                    sys_rst_n,            // 系统复位，低有效

    input                    enable,           // 通道使能信号
    input           [ 5: 0]  waveform,         // 波形类型选择，6位，范围0~63
    input   signed  [15: 0]  amplitude,        // 通道幅度控制，16位，范围0~65535，幅值为负时波形反相
    input   signed  [15: 0]  offset,           // 通道偏置控制，
    input           [47: 0]  freq_ctrl_word,   // 通道频率控制字，48位，范围0~2^48-1
    input           [47: 0]  phase_ctrl_word,  // 通道相位控制字，48位，范围0~2^48-1
    input           [15: 0]  duty_ctrl_word,  // 通道占空比控制字，16位，范围0~65535
    input           [31: 0]  slope_up_ctrl_word,  // 通道上升沿斜率控制字，16位，范围0~65535
    input           [31: 0]  slope_down_ctrl_word,  // 通道下降沿斜率控制字，16位，范围0~65535

    output  signed  [15: 0]  data_out          // 通道输出数据，16位，范围0~65535
);

localparam sine_wave = 6'd0;
localparam square_wave = 6'd1;
localparam triangle_wave = 6'd2;
localparam noise = 6'd3;
localparam dc = 6'd4;

wire                   channel_rst_n;  // 通道复位信号，低有效
wire  signed  [15: 0]  sine_out;  // 通道正弦波输出，16位，范围-32768~32767

wire signed  [47: 0]  phase_count_out;  // 通道相位计数器输出，48位，范围0~2^48-1

assign channel_rst_n = sys_rst_n & enable;  // 复位信号，低有效 

// 正弦波 DDS 实例化（高云 IP 核）
DDS_II_Top u_dds_ii_ch1 (
    .clk_i(sys_clk),                        // 输入时钟
    .rst_n_i(channel_rst_n),                    // 输入复位，低有效
    .phase_valid_i(channel_rst_n),              // 相位有效信号，始终为高电平
    .phase_inc_i(freq_ctrl_word),       // 频率控制字输入
    .phase_off_i(phase_ctrl_word),      // 相位控制字输入
    .phase_out_o(phase_count_out),          // 相位输出
    .sine_o(sine_out),                  // 正弦波输出
    .data_valid_o()                         // 数据有效信号
);

reg [15:0] phase_count;
always @(posedge sys_clk or negedge channel_rst_n) begin
    if (!channel_rst_n) begin
        phase_count <= 16'd0;
    end else begin
        phase_count <= phase_count_out[47:32];
    end
end

// 生成方波输出
wire [15:0] square_out = (phase_count >= duty_ctrl_word) ? 16'd32767 : -16'd32768;

// 生成三角波输出
reg [31:0] triangle_out;
reg [15:0] triangle_phase;

always @(posedge sys_clk or negedge channel_rst_n) begin
    if (!channel_rst_n) begin
        triangle_phase <= 16'd0;
    end else begin
        if (phase_count < duty_ctrl_word) begin
            triangle_phase <= phase_count;
        end else begin
            triangle_phase <= 16'd65535 - phase_count;
        end
    end
end

always @(posedge sys_clk or negedge channel_rst_n) begin
    if (!channel_rst_n) begin
        triangle_out <= 32'd0;
    end else if (phase_count < duty_ctrl_word) begin
        triangle_out <= triangle_phase * slope_up_ctrl_word;
    end else begin
        triangle_out <= triangle_phase * slope_down_ctrl_word;
    end
end

reg signed [15: 0] wave_mux;
always @(posedge sys_clk or negedge channel_rst_n) begin
    if (!channel_rst_n) begin
        wave_mux <= 16'd0;  // 复位时输出中间值（偏置为0）
    end else begin
        case(waveform)
            sine_wave: wave_mux <= sine_out;
            square_wave: wave_mux <= square_out;
            triangle_wave: wave_mux <= triangle_out[31:16] - 16'd32768; // 三角波输出为 Q16.16 格式，截取高 16 位 [31:16]，减去 32768 以使其范围为 -32768~32767
            dc: wave_mux <= 16'd0; 
        endcase
    end
end

// 使用 reg 的目的是为了防止组合逻辑过长导致的时序不收敛
// 乘积 (wave_mux * amplitude) 为 Q15.16 格式（1符号位，15位整数，16位小数）。
// 若直接截取高 16 位 [31:16]，等效于除以 2^16（缩放因子 65536）。
// 由于 amplitude 最大值为 32767（16bit有符号正数），直接截位会导致满幅输出减半。
// 因此左移 1 位（<<< 1），将缩放因子变为 32768（2^15），
// 确保 amplitude = 32767 时输出接近满幅 DAC 码值（±32767）。
reg signed [31:0] mult_out;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        mult_out <= 32'd0;
    end else begin
        mult_out <= (wave_mux * amplitude) <<< 1;
    end 
end

wire signed [15:0] right_shift = $signed(mult_out[31:16]);
assign data_out = right_shift + offset;

endmodule
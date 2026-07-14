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

module wave_generation_tb();

parameter SYS_CLK_FREQ = 150_000_000; // 系统时钟频率，单位Hz
parameter signed CH1_OFFSET = 0; // 通道1偏置，单位为16位有符号整数
parameter CH1_AMPLITUDE = 65535; // 通道1幅度，单位为16位无符号整数
parameter CH1_PHASE_CTRL_WORD = 48'd0; // 通道1相位控制字，单位为48位无符号整数

parameter _1MHz_FREQ_CTRL_WORD = 48'd1_876_499_844_737; // 1MHz频率控制字，单位为48位无符号整数
parameter _10MHz_FREQ_CTRL_WORD = 48'd18_764_998_447_377; // 10MHz频率控制字，单位为48位无符号整数

reg        clk;
reg        rst_n;
reg        ch1_enable;
reg [47:0] ch1_freq_ctrl_word;

wire [15:0] ch1_data_out;

wave_generation u_wave_generation (
    .sys_clk(clk),
    .sys_rst_n(rst_n),

    .ch1_enable(ch1_enable),
    .ch1_amplitude(ch1_amplitude),
    .ch1_offset(ch1_offset),
    .ch1_freq_ctrl_word(ch1_freq_ctrl_word),
    .ch1_phase_ctrl_word(CH1_PHASE_CTRL_WORD),

    .ch1_data_out(ch1_data_out)
);

initial begin
    clk = 1'b0;           
    forever #3.333 clk = ~clk;
end

initial begin
    rst_n = 0;
    ch1_enable = 0;
    ch1_freq_ctrl_word = _1MHz_FREQ_CTRL_WORD; // 1MHz 

    #100;
    rst_n = 1;
    ch1_enable = 1;

    # 5436
    ch1_enable = 0; // 禁用通道1
    # 500 
    ch1_enable = 1; // 重新启用通道1
    # 4064

    ch1_freq_ctrl_word = _10MHz_FREQ_CTRL_WORD; // 10MHz

    # 10000; // 等待一段时间以观察输出波形

    $stop;
end

endmodule
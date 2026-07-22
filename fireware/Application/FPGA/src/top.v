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

module ArbWave30 (
        // system ctrl
        input                clk_in_p,
        input                clk_in_n,
        input                ext_rst,

        // MCU communicate interface
        input                ctrl_spi_cs,
        input                ctrl_spi_clk,
        input                ctrl_spi_mosi,
        output               ctrl_spi_miso,

        input                ctrl_gpio1,
        input                ctrl_gpio2,

        // DAC data interface
        input                dco,
        output       [15:0]  ch1_data_out,
        // output reg  [15:0]  ch2_data_out,

        // DAC ctrl（DAC控制）
        output               dac_ctrl_rst_p,
        output               dac_ctrl_spi_cs,
        output               dac_ctrl_spi_clk,
        output               dac_ctrl_spi_mosi,
        input                dac_ctrl_spi_miso

        // AFE ctrl（模拟前端控制）
        // output  reg  [3:0]  AFE_Ctrl_ch1,
        // output  reg  [3:0]  AFE_Ctrl_ch2
    );

    parameter SYS_CLK_FREQ = 150_000_000; // 系统时钟频率，单位Hz

    wire                  sys_clk;
    wire                  sys_rst_n, dac_data_rst_n;
    wire                  pll_lock;

    reg                   ch1_enable;
    reg           [15:0]  ch1_amplitude;
    reg   signed  [15:0]  ch1_offset;
    reg           [47:0]  ch1_freq_ctrl_word;
    reg           [47:0]  ch1_phase_ctrl_word;
    wire          [15:0]  ch1_data;

    // 系统控制模块
    sys_ctrl u_sys_ctrl(
                 .clk_in_p(clk_in_p),
                 .clk_in_n(clk_in_n),
                 .data_clk(dco),
                 .ext_rst(ext_rst),

                 .sys_clk(sys_clk),
                 .sys_rst_n(sys_rst_n),
                 .dac_data_rst_n(dac_data_rst_n),
                 .pll_lock(pll_lock)
             );

    // 波形生成模块
    wave_generation ch1_wave_generation(
                        .sys_clk             	(sys_clk              ),
                        .sys_rst_n           	(sys_rst_n            ),
                        .enable          	(ch1_enable           ),
                        .amplitude       	(ch1_amplitude        ),
                        .offset          	(ch1_offset           ),
                        .freq_ctrl_word  	(ch1_freq_ctrl_word   ),
                        .phase_ctrl_word 	(ch1_phase_ctrl_word  ),
                        .data_out        	(ch1_data       )
                    );

    dac_interface #(
        .SYS_CLK_FREQ(SYS_CLK_FREQ)
    )u_dac_interface (
                      .sys_clk   	    (sys_clk      ),
                      .data_clk  	    (dco          ),
                      .sys_rst_n 	    (sys_rst_n    ),
                      .ch1_data_in   	(ch1_data     ),
                      .ch1_data_out  	(ch1_data_out ),
                      .rst_p            (dac_ctrl_rst_p),
                      .cs               (dac_ctrl_spi_cs  ),
                      .clk              (dac_ctrl_spi_clk  ),
                      .mosi             (dac_ctrl_spi_mosi  ),
                      .miso             (dac_ctrl_spi_miso )
                  );


    // 默认值
    always @(posedge sys_clk or negedge sys_rst_n) begin
        if (!sys_rst_n) begin
            ch1_enable <= 1'b1; // 通道1始终使能
            ch1_freq_ctrl_word <= 48'd1_876_499_844_737;
            ch1_phase_ctrl_word <= 48'd0; // 默认相位控制字为0
            ch1_amplitude <= 16'd32768; // 默认幅度为最大值
            ch1_offset <= 16'd16384; // 默认偏置为0s
        end
        else begin
            // 将来在这里插入 SPI 写入条件
            ch1_enable <= ch1_enable; // 通道1始终使能
            ch1_freq_ctrl_word <= ch1_freq_ctrl_word;
            ch1_phase_ctrl_word <= ch1_phase_ctrl_word; // 默认相位控制字为0
            ch1_amplitude <= ch1_amplitude; // 默认幅度为最大值
            ch1_offset <= ch1_offset; // 默认偏置为0s

        end
    end

endmodule

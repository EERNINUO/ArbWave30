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

        output               fpga_int,
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
    wire                  sys_rst_n, dco_rst_n;
    wire                  pll_lock;
    wire                  soft_reset;
    wire                  clock_source;

    // 系统控制模块
    sys_ctrl u_sys_ctrl(
                 .clk_in_p(clk_in_p),
                 .clk_in_n(clk_in_n),
                 .data_clk(dco),
                 .ext_rst(ext_rst),
                 .soft_rst(soft_reset),
                 .clock_source(clock_source),

                 .sys_clk(sys_clk),
                 .sys_rst_n(sys_rst_n),
                 .dco_rst_n(dco_rst_n),
                 .fpga_int(fpga_int),
                 .pll_lock(pll_lock)
             );

    // 波形生成模块
    wire                   ch1_enable;
    wire   signed  [5:0]  ch1_waveform;
    wire   signed  [15:0]  ch1_amplitude;
    wire   signed  [15:0]  ch1_offset;
    wire           [47:0]  ch1_freq_ctrl_word;
    wire           [15:0]  ch1_phase_ctrl_word;
    wire           [15:0]  ch1_duty_ctrl_word;
    wire   signed  [15:0]  ch1_data;

    wave_generation ch1_wave_generation(
                        .sys_clk            (sys_clk              ),
                        .sys_rst_n          (sys_rst_n            ),
                        .enable          	(ch1_enable           ),
                        .waveform           (ch1_waveform            ),
                        .amplitude       	(ch1_amplitude        ),
                        .offset          	(ch1_offset           ),
                        .freq_ctrl_word  	(ch1_freq_ctrl_word   ),
                        .phase_ctrl_word 	({ch1_phase_ctrl_word, 32'h00000000}  ),
                        .duty_ctrl_word  	(ch1_duty_ctrl_word   ),
                        .data_out        	(ch1_data       )
                    );

    dac_interface u_dac_interface (
                      .sys_clk   	    (sys_clk      ),
                      .data_clk  	    (dco          ),
                      .sys_rst_n 	    (sys_rst_n    ),
                      .ch1_data_in   	(ch1_data     ),
                      .ch1_data_out  	(ch1_data_out ),
                      .rst_p            (dac_ctrl_rst_p),
                      .cs               (dac_ctrl_spi_cs   ),
                      .clk              (dac_ctrl_spi_clk  ),
                      .mosi             (dac_ctrl_spi_mosi ),
                      .miso             (dac_ctrl_spi_miso )
                  );

    // SPI Slave 模块
    wire [6:0] spi_slave_addr;
    wire [15:0] spi_slave_rece_data;
    wire [15:0] spi_slave_trans_data;
    wire out_data_valid;

    wire address_error;
    
    spi_slave u_spi_slave(
        .sys_clk        	(sys_clk         ),
        .sys_rst_n      	(sys_rst_n       ),

        .spi_clk        	(ctrl_spi_clk        ),
        .spi_cs         	(ctrl_spi_cs        ),
        .spi_mosi       	(ctrl_spi_mosi        ),
        .spi_miso       	(ctrl_spi_miso        ),
        .addr           	(spi_slave_addr            ),
        .data_out       	(spi_slave_rece_data        ),
        .data_in        	(spi_slave_trans_data         ),
        .out_data_valid 	(out_data_valid  ),
        .address_error  	(address_error   )
    );
    

    // 寄存器模块

    register_block u_register_block(
        .sys_clk             	(sys_clk              ),
        .sys_rst_n           	(sys_rst_n            ),

        .addr                	(spi_slave_addr       ),
        .data_in                (spi_slave_rece_data  ),
        .data_out               (spi_slave_trans_data ),
        .write_en               (out_data_valid       ),
        .address_error          (address_error        ),

        .soft_reset          	(soft_reset           ),
        .clock_source        	(clock_source         ),

        .pll_lock            	(pll_lock             ),

        .ch1_enable          	(ch1_enable           ),
        .ch1_waveform        	(ch1_waveform         ),
        .ch1_freq_ctrl_word  	(ch1_freq_ctrl_word   ),
        .ch1_phase_ctrl_word 	(ch1_phase_ctrl_word  ),
        .ch1_ampl_ctrl_word   	(ch1_amplitude        ),
        .ch1_dc_offset_word  	(ch1_offset           ),
        .ch1_duty_ctrl_word  	(ch1_duty_ctrl_word),

        .ch2_enable          	(),
        .ch2_waveform        	(),
        .ch2_freq_ctrl_word  	(),
        .ch2_phase_ctrl_word 	(),
        .ch2_ampl_ctrl_word   	(),
        .ch2_dc_offset_word  	(),
        .ch2_duty_ctrl_word  	(),
        .dac_reg_addr        	(),
        .dac_reg_data        	(),
        .dac_reg_write       	()
    );

endmodule

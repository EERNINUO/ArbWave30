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
    input clk_in_p,
    input clk_in_n,
    input ext_rst,

    // MCU communicate interface 
    input ctrl_spi_cs,
    input ctrl_spi_sck,
    input ctrl_spi_mosi,
    output ctrl_spi_miso,

    input ctrl_gpio1,
    input ctrl_gpio2,

    // DAC data interface
    input dco,  
    output reg[15:0] data_ch1,
    // output reg[15:0] data_ch2,

    // DAC ctrl（DAC控制）
    output dac_ctrl_spi_cs,
    output dac_ctrl_spi_sck,
    output dac_ctrl_spi_mosi,
    input dac_ctrl_spi_miso

    // AFE ctrl（模拟前端控制）
    // output reg[3:0] AFE_Ctrl_ch1,
    // output reg[3:0] AFE_Ctrl_ch2
);

wire sys_clk, sys_rst_n;
wire pll_lock;

sys_ctrl sys_ctrl_inst(
    .clk_in_p(clk_in_p),
    .clk_in_n(clk_in_n),
    .ext_rst(ext_rst),

    .sys_clk(sys_clk),
    .sys_rst_n(sys_rst_n),
    .pll_lock(pll_lock)
);

endmodule

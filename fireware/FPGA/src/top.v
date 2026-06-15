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
    // system
    input sys_clk,
    input sys_rst_n,
    output sys_clk_rst,

    // MCU_Ctrl_interface 
    input ctrl_spi_cs,
    input ctrl_spi_sck,
    input ctrl_spi_mosi,
    output ctrl_spi_miso,
    input ctrl_gpio1,
    input ctrl_gpio2,

    // DAC_interface
    input dco,  
    output reg[15:0] data_ch1,
    output reg[15:0] data_ch2, // （CH2 数据接口，预留）

    // DAC_ctrl（DAC控制）
    output dac_ctrl_spi_cs,
    output dac_ctrl_spi_sck,
    output dac_ctrl_spi_mosi,
    input dac_ctrl_spi_miso,

    // AFE_ctrl（模拟前端控制，预留）
    output reg[3:0] AFE_Ctrl_ch1,
    output reg[3:0] AFE_Ctrl_ch2
);

endmodule
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

module sys_ctrl(
        input clk_in_p,
        input clk_in_n,
        input dco_clk,
        input ext_rst,

        output sys_clk,
        output sys_rst_n,
        output dac_data_rst_n,
        output pll_lock
    );

    wire pll_in;

    TLVDS_IBUF clk_buf(
                   .O(pll_in),

                   .I(clk_in_p),
                   .IB(clk_in_n)
               );

    Gowin_rPLL PLL(
                   .clkout(sys_clk), //output clkout

                   .lock(pll_lock), // pll
                   .clkin(pll_in) //input clkin
               );

    //============================
    // 异步复位同步释放（系统时钟域）
    //============================
    reg sys_rst_dly_1, sys_rst_dly_2;
    wire rst_n_a = ext_rst & pll_lock;  // 外部复位或PLL未锁定时都处于复位状态

    always @(posedge sys_clk or negedge rst_n_a) begin
        if (!rst_n_a) begin
            sys_rst_dly_1 <= 1'b0;
            sys_rst_dly_2 <= 1'b0;
        end
        else begin
            sys_rst_dly_1 <= 1'b1;
            sys_rst_dly_2 <= sys_rst_dly_1;
        end
    end

    assign sys_rst_n = sys_rst_dly_2;

    //============================
    // 异步复位同步释放（DAC时钟域）
    //============================
    reg dac_data_rst_dly_1, dac_data_rst_dly_2;

    always @(posedge dco_clk or negedge rst_n_a) begin
        if (!rst_n_a) begin
            dac_data_rst_dly_1 <= 1'b0;
            dac_data_rst_dly_2 <= 1'b0;
        end
        else begin
            dac_data_rst_dly_1 <= 1'b1;
            dac_data_rst_dly_2 <= dac_data_rst_dly_1;
        end
    end

    assign dac_data_rst_n = dac_data_rst_dly_2;

endmodule

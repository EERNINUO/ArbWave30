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

module register_block(
    input                        sys_clk,
    input                        sys_rst_n,

    input                [6:0]   addr,
    input                [15:0]  data_in,
    output  reg          [15:0]  data_out,
    input                        write_en,
    output  reg                  address_error,
    
    output                       soft_reset,
    output                       clock_source,

    input                        pll_lock,

    output                       ch1_enable,
    output               [5:0]   ch1_waveform,
    output               [47:0]  ch1_freq_ctrl_word,
    output               [15:0]  ch1_phase_ctrl_word,
    output       signed  [15:0]  ch1_ampl_ctrl_word,
    output       signed  [15:0]  ch1_dc_offset_word,
    output               [15:0]  ch1_duty_ctrl_word,
    output               [31:0]  ch1_slope_up_ctrl_word,
    output               [31:0]  ch1_slope_down_ctrl_word,

    output                       ch2_enable,
    output               [5:0]   ch2_waveform,
    output               [47:0]  ch2_freq_ctrl_word,
    output               [15:0]  ch2_phase_ctrl_word,
    output       signed  [15:0]  ch2_ampl_ctrl_word,
    output       signed  [15:0]  ch2_dc_offset_word,
    output               [15:0]  ch2_duty_ctrl_word,
    output               [31:0]  ch2_slope_up_ctrl_word,
    output               [31:0]  ch2_slope_down_ctrl_word,
    
    output  reg          [7:0]   dac_reg_addr,
    output  reg          [7:0]   dac_reg_data,
    output  reg                  dac_reg_write
);

// System control register
localparam  SOFT_RST       = 0;
localparam  CLOCK_SRC      = 1;
localparam  DAC_CMD_WRITE  = 2;
localparam  UPDATE         = 3;

// Channel control register
localparam  WAVEFORM_LSB   = 0;
localparam  WAVEFORM_MSB   = 5;
localparam  CHANNEL_ENABLE = 15;

// ============================================
// 1. 系统控制寄存器（0x02）
// ============================================
wire [15:0] sys_id_l = 15'h4257;
wire [15:0] sys_id_h = 15'h4152;
reg [15:0] sys_ctrl_reg;        // [3]:UPDATE, [2]:DAC_CMD, [1]:CLOCK_SRC, [0]:SOFT_RST
wire [15:0] sys_status = {15'b0, pll_lock};

// ============================================
// 2. 影子寄存器组（用于DDS参数，MCU写入后暂存于此）
// ============================================
// 通道1影子
reg [15:0] ch1_ctrl_shadow;          // 0x10
reg [15:0] ch1_freq_l_shadow;        // 0x11
reg [15:0] ch1_freq_m_shadow;        // 0x12
reg [15:0] ch1_freq_h_shadow;        // 0x13
reg [15:0] ch1_ampl_shadow;          // 0x14
reg [15:0] ch1_offset_shadow;        // 0x15
reg [15:0] ch1_phase_shadow;         // 0x16
reg [15:0] ch1_duty_shadow;          // 0x17
reg [15:0] ch1_slope_up_l_shadow;    // 0x18
reg [15:0] ch1_slope_up_h_shadow;    // 0x19
reg [15:0] ch1_slope_down_l_shadow;  // 0x1a
reg [15:0] ch1_slope_down_h_shadow;  // 0x1b

// 通道2影子
reg [15:0] ch2_ctrl_shadow;          // 0x20
reg [15:0] ch2_freq_l_shadow;        // 0x21
reg [15:0] ch2_freq_m_shadow;        // 0x22
reg [15:0] ch2_freq_h_shadow;        // 0x23
reg [15:0] ch2_ampl_shadow;          // 0x24
reg [15:0] ch2_offset_shadow;        // 0x25
reg [15:0] ch2_phase_shadow;         // 0x26
reg [15:0] ch2_duty_shadow;          // 0x27
reg [15:0] ch2_slope_down_l_shadow;  // 0x28
reg [15:0] ch2_slope_down_h_shadow;  // 0x29
reg [15:0] ch2_slope_up_l_shadow;    // 0x2a
reg [15:0] ch2_slope_up_h_shadow;    // 0x2b

// ============================================
// 3. 主寄存器组（影子加载后生效，DDS内核直接使用）
// ============================================
// 通道1主寄存器
reg [15:0] ch1_ctrl;          
reg [15:0] ch1_freq_l;        
reg [15:0] ch1_freq_m;        
reg [15:0] ch1_freq_h;        
reg [15:0] ch1_ampl;          
reg [15:0] ch1_offset;        
reg [15:0] ch1_phase;         
reg [15:0] ch1_duty;      
reg [15:0] ch1_slope_up_l;      
reg [15:0] ch1_slope_up_h;      
reg [15:0] ch1_slope_down_l;
reg [15:0] ch1_slope_down_h;

// 通道2主寄存器
reg [15:0] ch2_ctrl;          
reg [15:0] ch2_freq_l;        
reg [15:0] ch2_freq_m;        
reg [15:0] ch2_freq_h;        
reg [15:0] ch2_ampl;          
reg [15:0] ch2_offset;        
reg [15:0] ch2_phase;         
reg [15:0] ch2_duty;          
reg [15:0] ch2_slope_up_l;
reg [15:0] ch2_slope_up_h;
reg [15:0] ch2_slope_down_l;
reg [15:0] ch2_slope_down_h;

// ============================================
// 4. DAC缓存寄存器（0x30~0x35，供AD9747 SPI控制器使用）
// ============================================
reg [15:0] dac_cache_0;
reg [15:0] dac_cache_1; 
reg [15:0] dac_cache_2; 
reg [15:0] dac_cache_3; 
reg [15:0] dac_cache_4; 
reg [15:0] dac_cache_5; 

// write_en 上升沿检测
reg write_en_dly;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        write_en_dly <= 1'b0;
    end else begin
        write_en_dly <= write_en;
    end
end

assign write_en_rise = write_en & ~write_en_dly;

// SPI 寄存器写入
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        sys_ctrl_reg        <= 8'h0000;
        ch1_ctrl_shadow     <= 16'h0000;
        ch1_freq_l_shadow   <= 16'h0000;
        ch1_freq_m_shadow   <= 16'h0000;
        ch1_freq_h_shadow   <= 16'h0000;
        ch1_ampl_shadow     <= 16'h0000;
        ch1_offset_shadow   <= 16'h0000;
        ch1_phase_shadow    <= 16'h0000;
        ch1_duty_shadow     <= 16'h8000;
        ch1_slope_up_l_shadow   <= 16'h0000;
        ch1_slope_up_h_shadow   <= 16'h0002;
        ch1_slope_down_l_shadow <= 16'h0000;
        ch1_slope_down_h_shadow <= 16'h0002;
        ch2_ctrl_shadow     <= 16'h0000;
        ch2_freq_l_shadow   <= 16'h0000;
        ch2_freq_m_shadow   <= 16'h0000;
        ch2_freq_h_shadow   <= 16'h0000;
        ch2_ampl_shadow     <= 16'h0000;
        ch2_offset_shadow   <= 16'h0000;
        ch2_phase_shadow    <= 16'h0000;
        ch2_duty_shadow     <= 16'h8000;
        ch2_slope_up_l_shadow   <= 16'h0000;
        ch2_slope_up_h_shadow   <= 16'h0002;
        ch2_slope_down_l_shadow <= 16'h0000;
        ch2_slope_down_h_shadow <= 16'h0002;
        dac_cache_0  <= 16'h0000;
        dac_cache_1  <= 16'h0000;
        dac_cache_2  <= 16'hf901;
        dac_cache_3  <= 16'h0000;
        dac_cache_4  <= 16'hf901;
        dac_cache_5  <= 16'h0000;
    end else if (write_en_rise) begin
        case (addr)
            7'h02: sys_ctrl_reg <= data_in;
            7'h10: ch1_ctrl_shadow <= data_in;
            7'h11: ch1_freq_l_shadow <= data_in;
            7'h12: ch1_freq_m_shadow <= data_in;
            7'h13: ch1_freq_h_shadow <= data_in;
            7'h14: ch1_ampl_shadow <= data_in;
            7'h15: ch1_offset_shadow <= data_in;
            7'h16: ch1_phase_shadow <= data_in;
            7'h17: ch1_duty_shadow <= data_in;
            7'h18: ch1_slope_up_l_shadow <= data_in;
            7'h19: ch1_slope_up_h_shadow <= data_in;
            7'h1a: ch1_slope_down_l_shadow <= data_in;
            7'h1b: ch1_slope_down_h_shadow <= data_in;
            7'h20: ch2_ctrl_shadow <= data_in;
            7'h21: ch2_freq_l_shadow <= data_in;
            7'h22: ch2_freq_m_shadow <= data_in;
            7'h23: ch2_freq_h_shadow <= data_in;
            7'h24: ch2_ampl_shadow <= data_in;
            7'h25: ch2_offset_shadow <= data_in;
            7'h26: ch2_phase_shadow <= data_in;
            7'h27: ch2_duty_shadow <= data_in;
            7'h28: ch2_slope_up_l_shadow <= data_in;
            7'h29: ch2_slope_up_h_shadow <= data_in;
            7'h2a: ch2_slope_down_l_shadow <= data_in;
            7'h2b: ch2_slope_down_h_shadow <= data_in;
            // DAC 缓存区域
            7'h30: dac_cache_0 <= data_in;
            7'h31: dac_cache_1 <= data_in;
            7'h32: dac_cache_2 <= data_in;
            7'h33: dac_cache_3 <= data_in;
            7'h34: dac_cache_4 <= data_in;
            7'h35: dac_cache_5 <= data_in;
            default: ; // 非法地址忽略（ADDRESS_ERROR 在读取块处理）
        endcase
    end
    // UPDATE 自清零
    else if (sys_ctrl_reg[UPDATE] == 1'b1) begin
        sys_ctrl_reg[UPDATE] <= 1'b0;
    end
end

// 影子寄存器加载到主寄存器
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        ch1_ctrl     <= 16'h0000;
        ch1_freq_l   <= 16'h0000;
        ch1_freq_m   <= 16'h0000;
        ch1_freq_h   <= 16'h0000;
        ch1_ampl     <= 16'h0000;
        ch1_offset   <= 16'h0000;
        ch1_phase    <= 16'h0000;
        ch1_duty     <= 16'h8000;
        ch1_slope_up_l   <= 16'h0000;
        ch1_slope_up_h   <= 16'h0002;
        ch1_slope_down_l <= 16'h0000;
        ch1_slope_down_h <= 16'h0002;
        ch2_ctrl     <= 16'h0000;
        ch2_freq_l   <= 16'h0000;
        ch2_freq_m   <= 16'h0000;
        ch2_freq_h   <= 16'h0000;
        ch2_ampl      <= 16'h0000;
        ch2_offset   <= 16'h0000;
        ch2_phase    <= 16'h0000;
        ch2_duty     <= 16'h8000;
        ch2_slope_up_l   <= 16'h0000;
        ch2_slope_up_h   <= 16'h0002;
        ch2_slope_down_l <= 16'h0000;
        ch2_slope_down_h <= 16'h0002;
    end else if (sys_ctrl_reg[UPDATE]) begin
        // 批量更新所有影子寄存器 → 主寄存器
        ch1_ctrl     <= ch1_ctrl_shadow;
        ch1_freq_l   <= ch1_freq_l_shadow;
        ch1_freq_m   <= ch1_freq_m_shadow;
        ch1_freq_h   <= ch1_freq_h_shadow;
        ch1_ampl      <= ch1_ampl_shadow;
        ch1_offset   <= ch1_offset_shadow;
        ch1_phase    <= ch1_phase_shadow;
        ch1_duty     <= ch1_duty_shadow;
        ch1_slope_up_l   <= ch1_slope_up_l_shadow;
        ch1_slope_up_h   <= ch1_slope_up_h_shadow;
        ch1_slope_down_l <= ch1_slope_down_l_shadow;
        ch1_slope_down_h <= ch1_slope_down_h_shadow;
        ch2_ctrl     <= ch2_ctrl_shadow;
        ch2_freq_l   <= ch2_freq_l_shadow;
        ch2_freq_m   <= ch2_freq_m_shadow;
        ch2_freq_h   <= ch2_freq_h_shadow;
        ch2_ampl      <= ch2_ampl_shadow;
        ch2_offset   <= ch2_offset_shadow;
        ch2_phase    <= ch2_phase_shadow;
        ch2_duty     <= ch2_duty_shadow;
        ch2_slope_up_l   <= ch2_slope_up_l_shadow;
        ch2_slope_up_h   <= ch2_slope_up_h_shadow;
        ch2_slope_down_l <= ch2_slope_down_l_shadow;
        ch2_slope_down_h <= ch2_slope_down_h_shadow;
    end
end

// 读取寄存器
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        data_out <= 16'h0000;
        address_error <= 1'b0;
    end 
    else begin
        address_error <= 1'b0; // 默认地址合法
        case (addr)
            7'h00: data_out <= sys_id_l;               // SYS_ID_L (只读)
            7'h01: data_out <= sys_id_h;               // SYS_ID_H (只读)
            7'h02: data_out <= sys_ctrl_reg;
            7'h03: data_out <= {15'b0, pll_lock};
            7'h10: data_out <= ch1_ctrl;
            7'h11: data_out <= ch1_freq_l;
            7'h12: data_out <= ch1_freq_m;
            7'h13: data_out <= ch1_freq_h;
            7'h14: data_out <= ch1_ampl;
            7'h15: data_out <= ch1_offset;
            7'h16: data_out <= ch1_phase;
            7'h17: data_out <= ch1_duty;
            7'h20: data_out <= ch2_ctrl;
            7'h21: data_out <= ch2_freq_l;
            7'h22: data_out <= ch2_freq_m;
            7'h23: data_out <= ch2_freq_h;
            7'h24: data_out <= ch2_ampl;
            7'h25: data_out <= ch2_offset;
            7'h26: data_out <= ch2_phase;
            7'h27: data_out <= ch2_duty;
            7'h30: data_out <= dac_cache_0;
            7'h31: data_out <= dac_cache_1;
            7'h32: data_out <= dac_cache_2;
            7'h33: data_out <= dac_cache_3;
            7'h34: data_out <= dac_cache_4;
            7'h35: data_out <= dac_cache_5;
            default begin
                data_out <= 16'h0000;
                address_error <= 1'b1; // 非法地址
            end
        endcase
    end
end

// ---- SYS_CTRL 输出 ----
assign soft_reset = sys_ctrl_reg[SOFT_RST];           // SOFT_RST
assign clock_source = sys_ctrl_reg[CLOCK_SRC];         // CLOCK_SRC

// ---- 通道1 输出（主寄存器驱动） ----
assign ch1_enable = ch1_ctrl[CHANNEL_ENABLE];
assign ch1_waveform = ch1_ctrl[WAVEFORM_MSB:WAVEFORM_LSB];
assign ch1_freq_ctrl_word = {ch1_freq_h[15:0], ch1_freq_m[15:0], ch1_freq_l[15:0]};
assign ch1_phase_ctrl_word = ch1_phase[15:0];
assign ch1_ampl_ctrl_word = ch1_ampl[15:0];
assign ch1_dc_offset_word = ch1_offset[15:0];
assign ch1_duty_ctrl_word = ch1_duty[15:0];   // 低10位有效
assign ch1_slope_up_ctrl_word = {ch1_slope_up_h[15:0], ch1_slope_up_l[15:0]};
assign ch1_slope_down_ctrl_word = {ch1_slope_down_h[15:0], ch1_slope_down_l[15:0]};

// ---- 通道2 输出（主寄存器驱动） ----
assign ch2_enable = ch2_ctrl[CHANNEL_ENABLE];
assign ch2_waveform = ch2_ctrl[WAVEFORM_MSB:WAVEFORM_LSB];
assign ch2_freq_ctrl_word = {ch2_freq_h[15:0], ch2_freq_m[15:0], ch2_freq_l[15:0]};
assign ch2_phase_ctrl_word = ch2_phase[15:0];
assign ch2_ampl_ctrl_word = ch2_ampl[15:0];
assign ch2_dc_offset_word = ch2_offset[15:0];
assign ch2_duty_ctrl_word = ch2_duty[15:0];
assign ch2_slope_up_ctrl_word = {ch2_slope_up_h[15:0], ch2_slope_up_l[15:0]};
assign ch2_slope_down_ctrl_word = {ch2_slope_down_h[15:0], ch2_slope_down_l[15:0]};

// ---- DAC 缓存输出（主缓存驱动） ----
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        dac_reg_addr <= 8'h00;
        dac_reg_data <= 8'h00;
        dac_reg_write <= 1'b0;
    end
    else begin
        // TODO
    end
end

endmodule
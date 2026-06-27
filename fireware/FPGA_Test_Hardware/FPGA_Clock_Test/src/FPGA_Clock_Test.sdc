//Copyright (C)2014-2026 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//Tool Version: V1.9.12.01 (64-bit) 
//Created Time: 2026-06-27 10:42:57
create_clock -name clk_in -period 20 -waveform {0 10} [get_ports {sys_clk_t}]
create_clock -name sys_clk -period 6.667 -waveform {0 3.333} [get_nets {clk}]

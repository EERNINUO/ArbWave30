//Copyright (C)2014-2026 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//Tool Version: V1.9.12.01 (64-bit) 
//Created Time: 2026-06-29 15:46:10
create_clock -name clk_in -period 20 -waveform {0 10} [get_ports {clk_in_p}]
create_clock -name data_out_clk -period 6.667 -waveform {0 3.333} [get_ports {dco}]

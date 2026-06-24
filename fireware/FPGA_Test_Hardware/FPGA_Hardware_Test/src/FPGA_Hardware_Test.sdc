//Copyright (C)2014-2026 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//Tool Version: V1.9.12.01 (64-bit) 
//Created Time: 2026-06-24 23:06:33
create_clock -name sys_clk -period 40 -waveform {0 20} [get_ports {LED}]

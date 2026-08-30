//Copyright (C)2014-2026 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//Tool Version: V1.9.12.01 (64-bit) 
//Created Time: 2026-07-19 23:33:34
create_clock -name clk_in -period 20 -waveform {0 10} [get_ports {clk_in_p}]
create_clock -name data_out_clk -period 6.667 -waveform {0 3.333} [get_ports {dco}]
set_clock_groups -asynchronous -group [get_clocks {clk_in}] -group [get_clocks {data_out_clk}]
set_output_delay -clock data_out_clk -2.8 -max [get_ports {ch1_data_out[15] ch1_data_out[14] ch1_data_out[13] ch1_data_out[12] ch1_data_out[11] ch1_data_out[10] ch1_data_out[9] ch1_data_out[8] ch1_data_out[7] ch1_data_out[6] ch1_data_out[5] ch1_data_out[4] ch1_data_out[3] ch1_data_out[2] ch1_data_out[1] ch1_data_out[0]}]

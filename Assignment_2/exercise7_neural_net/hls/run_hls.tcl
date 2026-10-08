set script_dir [file dirname [file normalize [info script]]]
set project_dir [file join $script_dir neuralNet_hls]

open_project -reset $project_dir
set_top neuralNet
add_files [file join $script_dir neuralNet.cpp]
add_files -tb [file join $script_dir testbench.cpp]

open_solution -reset baseline -flow_target vivado
set_part {xc7z010clg400-1}
create_clock -period 10 -name default
config_compile -pipeline_loops 0
csim_design
csynth_design

open_solution -reset pipelined -flow_target vivado
set_part {xc7z010clg400-1}
create_clock -period 10 -name default
config_compile -pipeline_loops 0
set_directive_pipeline -II 1 "neuralNet/input_dot_product"
set_directive_pipeline -II 1 "neuralNet/output_dot_product"
csim_design
csynth_design
cosim_design -rtl vhdl
export_design -format ip_catalog -rtl vhdl -version 1.0.1

exit

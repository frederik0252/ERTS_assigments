set script_dir [file dirname [file normalize [info script]]]
set project_dir [file join $script_dir Ex7]
set bit_file [file join $project_dir Ex7.runs impl_1 system_wrapper.bit]
open_project [file join $project_dir Ex7.xpr]
open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target
set device [lindex [get_hw_devices xc7z010_1] 0]
current_hw_device $device
refresh_hw_device $device
set_property PROGRAM.FILE $bit_file $device
program_hw_devices $device
refresh_hw_device $device

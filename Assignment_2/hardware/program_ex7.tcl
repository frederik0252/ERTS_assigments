set script_dir [file dirname [file normalize [info script]]]
set bit_file [file normalize [file join $script_dir Ex7 Ex7.runs impl_1 system_wrapper.bit]]
open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target
set device [lindex [get_hw_devices xc7z010_1] 0]
if {$device eq ""} {
    error "xc7z010_1 was not found"
}
current_hw_device $device
refresh_hw_device $device
set_property PROGRAM.FILE $bit_file $device
program_hw_devices $device
refresh_hw_device $device
puts "CODEX_PROGRAM_DEVICE=$device"
puts "CODEX_PROGRAM_FILE=[get_property PROGRAM.FILE $device]"
puts "CODEX_PROGRAM_DONE=[get_property REGISTER.IR.BIT5_DONE $device]"
close_hw_target
disconnect_hw_server
exit

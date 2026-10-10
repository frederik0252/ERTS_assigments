set script_dir [file dirname [file normalize [info script]]]
set project_dir [file join $script_dir Ex7]
set project_file [file join $project_dir Ex7.xpr]
set new_ip_repo [file normalize [file join $script_dir .. exercise7_neural_net hls neuralNet_hls pipelined impl ip]]
set old_ip_repo [file normalize [file join $project_dir Ex7.ipdefs ip_0]]

open_project $project_file
set_property ip_repo_paths [list $new_ip_repo] [current_project]
update_ip_catalog

open_bd_design [file join $project_dir Ex7.srcs sources_1 bd system system.bd]
set_property -dict [list CONFIG.GPIO_BOARD_INTERFACE {Custom} CONFIG.USE_BOARD_FLOW {false} CONFIG.C_ALL_INPUTS {1} CONFIG.C_GPIO_WIDTH {4}] [get_bd_cells buttons]
foreach ip [get_ips -quiet *] {
    if {[catch {upgrade_ip $ip} upgrade_error]} {
        puts "CODEX_UPGRADE_WARNING=$ip: $upgrade_error"
    }
}
connect_bd_net [get_bd_ports inSwitch] [get_bd_pins neuralNet_0/inSwitch]
connect_bd_net [get_bd_ports outLeds] [get_bd_pins neuralNet_0/outLeds]
if {[llength [get_bd_intf_nets -quiet -of_objects [get_bd_intf_pins neuralNet_0/s_axi_BUS_r]]] == 0} {
    apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0"} [get_bd_intf_pins neuralNet_0/s_axi_BUS_r]
}
assign_bd_address
validate_bd_design
save_bd_design
generate_target all [get_files system.bd]

reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1

set impl_status [get_property STATUS [get_runs impl_1]]
puts "CODEX_IMPL_STATUS=$impl_status"
if {![string match "*Complete*" $impl_status]} {
    error "Implementation did not complete: $impl_status"
}

open_run impl_1
report_timing_summary -file [file join $script_dir ex7_timing_summary.rpt]
report_utilization -file [file join $script_dir ex7_utilization.rpt]
set bit_file [file join $project_dir Ex7.runs impl_1 system_wrapper.bit]
set xsa_file [file join $script_dir Ex7_2026_1.xsa]
write_hw_platform -fixed -include_bit -force -file $xsa_file
puts "CODEX_BITSTREAM=$bit_file"
puts "CODEX_XSA=$xsa_file"
close_project
exit

# Migrate the supplied Ex5 platform (Ex3 platform plus matrix IP) for Ex3--5.
# Run sequentially; this creates no GUI. Source archive is left unchanged.
set script_dir [file dirname [file normalize [info script]]]
set archive [file normalize [file join /home firas Downloads Ex5.xpr.zip]]
if {[llength $argv] > 0} { set archive [file normalize [lindex $argv 0]] }
if {![file exists $archive]} { error "Supply the course Ex5.xpr.zip path" }
set generated [file join $script_dir generated_ex345]
set starter_dir [file join $generated Ex5]
set project_dir [file join $generated Ex345]
set project_file [file join $project_dir Ex345.xpr]
set archive_hash [lindex [exec sha256sum $archive] 0]
set archive_stamp [file join $generated course_archive.sha256]
if {[file exists $archive_stamp]} {
    set stamp_fd [open $archive_stamp r]
    set previous_hash [string trim [read $stamp_fd]]
    close $stamp_fd
    if {$previous_hash ne $archive_hash} {
        error "Course archive changed; preserve/rename generated_ex345 before rebuilding"
    }
} elseif {[file exists $starter_dir] || [file exists $project_dir]} {
    error "Existing generated_ex345 has no archive provenance; preserve/rename it before rebuilding"
}
file mkdir $generated
set stamp_fd [open $archive_stamp w]
puts $stamp_fd $archive_hash
close $stamp_fd
puts "ERTS_SOURCE_ARCHIVE_SHA256=$archive_hash"
exec unzip -n -q $archive Ex5/Ex5.srcs/sources_1/bd/system/hw_handoff/system_bd.tcl Ex5/Ex5.ipdefs/* -d $generated
set_param general.maxThreads 2
if {[file exists $project_file]} {
    open_project $project_file
} else {
    create_project Ex345 $project_dir -part xc7z010clg400-1
    set_property target_language VHDL [current_project]
}
set_property ip_repo_paths [list [file join $starter_dir Ex5.ipdefs led_ip_1.0_0] [file join $starter_dir Ex5.ipdefs matrix_ip_1.0_0]] [current_project]
update_ip_catalog
if {[llength [get_files -quiet system.bd]] == 0} {
    # Recreate from the provided BD Tcl using current catalog revisions. This
    # avoids the old locked board-preset output products; no source IP is changed.
    set source_fd [open [file join $starter_dir Ex5.srcs sources_1 bd system hw_handoff system_bd.tcl] r]
    set original_tcl [read $source_fd]
    close $source_fd
    set migrated_tcl [string map [list \
        {set scripts_vivado_version 2017.2} {set scripts_vivado_version 2026.1} \
        {CONFIG.GPIO_BOARD_INTERFACE {btns_4bits}} {CONFIG.GPIO_BOARD_INTERFACE {Custom}} \
        {CONFIG.GPIO_BOARD_INTERFACE {sws_4bits}} {CONFIG.GPIO_BOARD_INTERFACE {Custom}} \
        {CONFIG.USE_BOARD_FLOW {true}} {CONFIG.USE_BOARD_FLOW {false}}] $original_tcl]
    eval $migrated_tcl
} else {
    open_bd_design [get_files system.bd]
}
set constraints [get_files -quiet -of_objects [get_filesets constrs_1]]
if {[llength $constraints]} { remove_files $constraints }
add_files -fileset constrs_1 -norecurse [file join $script_dir ex345_zybo.xdc]
validate_bd_design
save_bd_design
generate_target all [get_files system.bd]
add_files -norecurse [make_wrapper -files [get_files system.bd] -top]
set_property top system_wrapper [current_fileset]
update_compile_order -fileset sources_1
reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 2
wait_on_run impl_1
set status [get_property STATUS [get_runs impl_1]]
puts "ERTS_EX345_IMPLEMENTATION=$status"
if {![string match "*write_bitstream Complete*" $status]} {
    error "Ex345 bitstream build failed: $status"
}
open_run impl_1
report_timing_summary -file [file join $script_dir ex345_timing_summary.rpt]
report_utilization -file [file join $script_dir ex345_utilization.rpt]
report_drc -file [file join $script_dir ex345_drc.rpt]
write_hw_platform -fixed -include_bit -force -file [file join $script_dir Ex345_2026_1.xsa]
open_bd_design [get_files system.bd]
write_bd_tcl -force [file join $script_dir ex345_system_2026.tcl]
puts "ERTS_EX345_BITSTREAM=[file join $project_dir Ex345.runs impl_1 system_wrapper.bit]"
close_project
exit

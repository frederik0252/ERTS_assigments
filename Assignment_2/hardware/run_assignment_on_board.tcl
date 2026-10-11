# Volatile JTAG programming only. Select one known cable, never an arbitrary target.
set script_dir [file dirname [file normalize [info script]]]
if {[llength $argv] < 1} { error "Usage: xsdb run_assignment_on_board.tcl 3|4|5|7 ?cable_serial?" }
set exercise [lindex $argv 0]
set cable 210279655163A
if {[llength $argv] > 1} { set cable [lindex $argv 1] }
if {![regexp {^[A-Za-z0-9_-]+$} $cable]} { error "Invalid cable serial" }
switch -- $exercise {
    3 { set app ex3_console }
    4 { set app ex4_matrix }
    5 { set app ex5_matrix }
    7 { set app neuralNetTest }
    default { error "Exercise must be 3, 4, 5 or 7" }
}
if {$exercise == 7} {
    set bit [file join $script_dir Ex7 Ex7.runs impl_1 system_wrapper.bit]
    set workspace [file join $script_dir Ex7_vitis]
} else {
    set bit [file join $script_dir generated_ex345 Ex345 Ex345.runs impl_1 system_wrapper.bit]
    set workspace [file join $script_dir Ex345_vitis]
}
set elf [file join $workspace $app build ${app}.elf]
set psinit [file join $workspace $app _ide psinit ps7_init.tcl]
foreach artifact [list $bit $elf $psinit] {
    if {![file isfile $artifact]} { error "Missing native artifact: $artifact" }
}
connect -url tcp:127.0.0.1:3121
set found 0
foreach props [targets -target-properties] {
    if {[dict get $props name] eq "xc7z010" && [dict exists $props jtag_cable_serial] &&
        [dict get $props jtag_cable_serial] eq $cable} { incr found }
}
if {$found != 1} { error "Expected exactly one XC7Z010 on cable $cable, found $found" }
puts "ERTS_BOARD_CABLE=$cable"
puts "ERTS_BOARD_EXERCISE=$exercise"
puts "ERTS_BOARD_BITSTREAM=$bit"
puts "ERTS_BOARD_ELF=$elf"
targets -set -filter [format {name == "APU" && jtag_cable_serial == "%s"} $cable]
rst -system
after 1500
fpga -file $bit
after 500
targets -set -filter [format {name == "ARM Cortex-A9 MPCore #0" && jtag_cable_serial == "%s"} $cable]
rst -processor
source $psinit
ps7_init
ps7_post_config
dow $elf
con
after 3000
disconnect
exit

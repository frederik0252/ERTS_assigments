set script_dir [file dirname [file normalize [info script]]]
set bit_file [file normalize [file join $script_dir Ex7 Ex7.runs impl_1 system_wrapper.bit]]
set elf_file [file normalize [file join $script_dir Ex7_vitis neuralNetTest build neuralNetTest.elf]]
set ps7_init_file [file normalize [file join $script_dir Ex7_vitis neuralNetTest _ide psinit ps7_init.tcl]]

foreach required_file [list $bit_file $elf_file $ps7_init_file] {
    if {![file exists $required_file]} {
        error "Required board-run artifact is missing: $required_file"
    }
}

connect -url tcp:127.0.0.1:3121

targets -set -nocase -filter {name =~ "APU"}
rst -system
after 1500

fpga -file $bit_file
after 500

targets -set -nocase -filter {name =~ "ARM Cortex-A9 MPCore #0"}
rst -processor
source $ps7_init_file
ps7_init
ps7_post_config

dow $elf_file
puts "CODEX_BOARD_ELF=$elf_file"
puts "CODEX_BOARD_TARGET=[targets -target-properties]"
con
after 3000
disconnect
exit

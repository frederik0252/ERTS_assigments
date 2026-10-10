set script_dir [file dirname [file normalize [info script]]]
open_project [file join $script_dir Ex7 Ex7.xpr]
open_bd_design [file join $script_dir Ex7 Ex7.srcs sources_1 bd system system.bd]

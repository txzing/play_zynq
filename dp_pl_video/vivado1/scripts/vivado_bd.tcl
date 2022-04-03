set BD_SRC [lindex $argv 0]
set BOARD [lindex $argv 1]
set IP_SRC [lindex $argv 2]
set ip_cache_path ip_cache

set outputDir build

if {[string equal ${BOARD} "ultra96v2"]} {
    create_project -part xczu3eg-sbva484-1-e -in_memory
    set_property board_part em.avnet.com:ultra96v2:part0:1.0 [current_project]	
} elseif {[string equal ${BOARD} "kv260"]} {
    create_project -part xck26-sfvc784-2LV-c -in_memory
    set_property board_part xilinx.com:kv260_som:part0:1.2 [current_project]
    set_property board_connections {som240_1_connector xilinx.com:kv260_carrier:som240_1_connector:1.2} [current_project]
} elseif {[string equal ${BOARD} "ACQ_V2"]} {
    create_project -part xczu2cg-sfvc784-1-e -in_memory
}
set_property target_language Verilog [current_project]
set_property default_lib work [current_project]

set_property ip_repo_paths $IP_SRC [current_project]
#config_ip_cache -use_cache_location $ip_cache_path
update_ip_catalog

source ${BD_SRC}

save_bd_design_as -dir $outputDir -force system.bd

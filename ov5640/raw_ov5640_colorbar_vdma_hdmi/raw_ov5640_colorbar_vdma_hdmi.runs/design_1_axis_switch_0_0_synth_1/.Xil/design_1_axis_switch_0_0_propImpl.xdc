set_property SRC_FILE_INFO {cfile:/opt/Xilinx/Vivado/2018.3/data/ip/xpm/xpm_cdc/tcl/xpm_cdc_handshake.tcl rfile:../../../../../../../../opt/Xilinx/Vivado/2018.3/data/ip/xpm/xpm_cdc/tcl/xpm_cdc_handshake.tcl id:1 order:LATE scoped_inst:inst/gen_static_router.gen_synch.inst_cdc_handshake/inst_xpm_cdc_handshake unmanaged:yes} [current_design]
current_instance inst/gen_static_router.gen_synch.inst_cdc_handshake/inst_xpm_cdc_handshake
set_property src_info {type:SCOPED_XDC file:1 line:30 export:INPUT save:NONE read:READ} [current_design]
create_waiver -internal -type CDC -id {CDC-15} -user "xpm_cdc" -desc "The CDC-15 warning is waived as it is safe in the context of XPM_CDC_HANDSHAKE." -from [get_pins -quiet {src_hsdata_ff_reg*/C}] -to [get_pins -quiet {dest_hsdata_ff_reg*/D}]

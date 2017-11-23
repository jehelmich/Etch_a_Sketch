# TCL File Generated by Component Editor 16.1
# Thu Nov 23 11:42:04 GMT 2017
# DO NOT MODIFY


# 
# RotaryCtl2 "RotaryCtl2" v1.0
#  2017.11.23.11:42:04
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module RotaryCtl2
# 
set_module_property DESCRIPTION ""
set_module_property NAME RotaryCtl2
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME RotaryCtl2
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL rotary
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file debounce.sv SYSTEM_VERILOG PATH source-files/debounce.sv
add_fileset_file rotary.sv SYSTEM_VERILOG PATH source-files/rotary.sv TOP_LEVEL_FILE


# 
# parameters
# 


# 
# display items
# 


# 
# connection point clock
# 
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clk clk Input 1


# 
# connection point rotary_event
# 
add_interface rotary_event conduit end
set_interface_property rotary_event associatedClock clock
set_interface_property rotary_event associatedReset ""
set_interface_property rotary_event ENABLED true
set_interface_property rotary_event EXPORT_OF ""
set_interface_property rotary_event PORT_NAME_MAP ""
set_interface_property rotary_event CMSIS_SVD_VARIABLES ""
set_interface_property rotary_event SVD_ADDRESS_GROUP ""

add_interface_port rotary_event rot_ccw rotary_ccw Output 1
add_interface_port rotary_event rot_cw rotary_cw Output 1


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset rst reset Input 1


# 
# connection point rotary_pos
# 
add_interface rotary_pos conduit end
set_interface_property rotary_pos associatedClock ""
set_interface_property rotary_pos associatedReset ""
set_interface_property rotary_pos ENABLED true
set_interface_property rotary_pos EXPORT_OF ""
set_interface_property rotary_pos PORT_NAME_MAP ""
set_interface_property rotary_pos CMSIS_SVD_VARIABLES ""
set_interface_property rotary_pos SVD_ADDRESS_GROUP ""

add_interface_port rotary_pos rotary_pos export Output 8


# 
# connection point rotary_in
# 
add_interface rotary_in conduit end
set_interface_property rotary_in associatedClock clock
set_interface_property rotary_in associatedReset ""
set_interface_property rotary_in ENABLED true
set_interface_property rotary_in EXPORT_OF ""
set_interface_property rotary_in PORT_NAME_MAP ""
set_interface_property rotary_in CMSIS_SVD_VARIABLES ""
set_interface_property rotary_in SVD_ADDRESS_GROUP ""

add_interface_port rotary_in rotary_in rotary_in Input 2


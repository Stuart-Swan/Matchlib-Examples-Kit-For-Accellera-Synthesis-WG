
# Establish the location of this script and use it to reference all
# other files in this example
set sfd [file dirname [info script]]

# Reset the options to the factory defaults
options defaults
project new

# Set simulation options (no VHDL model for DPRAM)
options set /Input/CppStandard c++11
options set /Input/CompilerFlags {-DBANKED_MULTI_PROCESS -DSC_INCLUDE_DYNAMIC_PROCESSES }
options set /Input/SearchPath {../../include} -append
options set /Input/SearchPath {$MGC_HOME/shared/pkgs/matchlib/cmod/include} -append
options set /Output/OutputVHDL false
flow package require /SCVerify

directive set -SYSC_CHANOPER_FLOW true

# Read Design and Libraries
solution file add ${sfd}/testbench.cpp -type C++
solution library add nangate-45nm_beh
solution library add ccs_sample_mem
directive set -MEM_MAP_THRESHOLD 16
go libraries

# Set clock period
directive set -CLOCKS {clk {-CLOCK_PERIOD 2}}
go assembly

# Generate RTL
go extract


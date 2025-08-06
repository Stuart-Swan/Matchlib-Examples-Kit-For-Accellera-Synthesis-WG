set sfd [file dir [info script]]

options defaults
options set /Input/CppStandard c++11
options set /Input/CompilerFlags " "
options set /Input/SearchPath {$MGC_HOME/shared/pkgs/matchlib/cmod/include} -append
options set /Input/SearchPath $sfd/../../include -append
options set /Input/SearchPath "$sfd/RAM_lib" -append
options set /ComponentLibs/SearchPath "$sfd/RAM_lib" -append

project new

flow package require /SCVerify
solution file add "$sfd/dut.h" -type CHEADER
solution file add "$sfd/testbench.cpp" -type C++ -exclude true

go analyze
solution design set dut -top
go compile
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add RAM_1R1W

go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0}}

go assembly
go architect
go allocate
go extract


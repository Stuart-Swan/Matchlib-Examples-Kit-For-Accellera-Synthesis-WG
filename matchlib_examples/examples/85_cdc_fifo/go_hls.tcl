set sfd [file dir [info script]]

options defaults

options set /Input/CppStandard c++11
options set /Input/CompilerFlags " "
options set /Input/SearchPath {../../include} -append
options set /Input/SearchPath {$MGC_HOME/shared/pkgs/matchlib/cmod/include} -append

project new

flow package require /SCVerify
flow package require /QuestaSIM
flow package option set /QuestaSIM/ENABLE_CODE_COVERAGE true
flow package option set /QuestaSIM/MSIM_DOFILE msim.do

solution file add "$sfd/testbench.cpp" -type C++

go analyze
solution design set dut -top

go compile
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm

go libraries
directive set -CLOCKS {clk1 {-CLOCK_PERIOD 2.0} clk2 {-CLOCK_PERIOD 2.0}}

go assembly
go architect
go allocate
go extract


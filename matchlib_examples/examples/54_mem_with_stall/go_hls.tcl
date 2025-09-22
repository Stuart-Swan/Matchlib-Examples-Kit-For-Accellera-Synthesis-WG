project new
solution options defaults
flow package require /SCVerify
options set /Input/SearchPath {../../include} -append
options set /Input/SearchPath {$MGC_HOME/shared/pkgs/matchlib/cmod/include} -append
solution options set /Input/CompilerFlags {-DFORCE_AUTO_PORT=Connections::DIRECT_PORT}
directive set -ON_THE_FLY_PROTOTYPING false
solution file add ./dut.cpp -type C++
solution file add ./testbench.cpp -type C++ -exclude true
directive set -STRUCT_LAYOUT c_style
directive set -PRESERVE_STRUCTS true
directive set -STRICT_CHANSYNC_IO_SCHEDULING true 
go analyze
solution design set dut -top
go compile
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0 -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0}}
go assembly
directive set /dut/dut:ProcessingThread/ProcessingThread -STRICT_MIO_SCHEDULING false
go architect
directive set /dut/dut:ProcessingThread/ProcessingThread/mem1_xact.rsp_chan.Pop() -CSTEPS_FROM {{mem1_xact.rd_req_chan.Push() == 1}}
directive set /dut/dut:ProcessingThread/ProcessingThread/mem2_xact.rsp_chan.Pop()#1 -CSTEPS_FROM {{mem2_xact.rd_req_chan.Push()#1 == 1}}
directive set /dut/dut:ProcessingThread/ProcessingThread/mem1_xact.rsp_chan.Pop()#2 -CSTEPS_FROM {{mem1_xact.rd_req_chan.Push()#2 == 1}}
directive set /dut/dut:ProcessingThread/ProcessingThread/mem2_xact.rsp_chan.Pop()#3 -CSTEPS_FROM {{mem2_xact.rd_req_chan.Push()#3 == 1}}
go allocate
go extract

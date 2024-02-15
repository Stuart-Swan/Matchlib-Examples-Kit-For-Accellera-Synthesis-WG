// INSERT_EULA_COPYRIGHT: 2020

#include "crossed_flops.h"
#include <mc_scverify.h>
using namespace::std;

class Top : public sc_module
{
public:
  CCS_DESIGN(crossed_flops) CCS_INIT_S1(crossed_flops1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);
  SC_SIG(uint32_t, out0);
  SC_SIG(uint32_t, out1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    crossed_flops1.clk(clk);
    crossed_flops1.rst_bar(rst_bar);
    crossed_flops1.out0(out0);
    crossed_flops1.out1(out1);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    sc_object_tracer<sc_clock> trace_clk(clk);
  }

  void stim() {
    CCS_LOG("Stimulus started");
    rst_bar = 0;
    wait(5);

    // take dut out of reset
    rst_bar = 1;

    wait(10);
    CCS_LOG("stopping sim at end of stimulus");
    sc_stop();
    wait(5);
  }
};


int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  Top top("top");
  trace_hierarchy(&top, trace_file_ptr);
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


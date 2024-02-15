// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>

class testbench : public sc_module
{
public:
  CCS_DESIGN(dut) CCS_INIT_S1(dut1);

  sc_clock clk1;
  sc_clock clk2;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<uint32_t>        CCS_INIT_S1(out1);
  Connections::Combinational<uint32_t>        CCS_INIT_S1(in1);

  SC_CTOR(testbench)
    :   clk1("clk1", 3, SC_NS, 0.5,0,SC_NS,true)
    ,   clk2("clk2", 1, SC_NS, 0.5,0,SC_NS,true)
    {
    sc_object_tracer<sc_clock> trace_clk1(clk1);
    sc_object_tracer<sc_clock> trace_clk2(clk2);

    dut1.clk1(clk1);
    dut1.clk2(clk2);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);

    SC_CTHREAD(reset, clk1);

    SC_THREAD(stim);
    sensitive << clk1.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk2.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }


  void stim() {
    CCS_LOG("Stimulus started");
    in1.ResetWrite();
    wait();

    for (int i = 0; i < 10; i++) {
      in1.Push(i);
    }

    wait(50);
    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      CCS_LOG("TB resp sees: " << std::hex << out1.Pop());
    }
  }

  void reset() {
    rst_bar.write(0);
    wait();
    rst_bar.write(1);
    wait();
  }
};

int sc_main(int argc, char *argv[])
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  testbench top("top");
  channel_logs logs;
  logs.enable("chan_log", true);
  logs.log_hierarchy(top);
  trace_hierarchy(&top, trace_file_ptr);
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


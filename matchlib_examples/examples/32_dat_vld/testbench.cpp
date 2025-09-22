// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>
#include <stable_random.h>
#include <memory>

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  SC_SIG(bool,   in_vld);
  SC_SIG(uint32, in_dat);
  SC_SIG(bool,   out_vld);
  SC_SIG(uint32, out_dat);

  SC_CTOR(Top)
    :   clk ("clk",  1, SC_NS, 0.5,0,SC_NS,true) 
  {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1.vld(in_vld);
    dut1.in1.dat(in_dat);
    dut1.out1.vld(out_vld);
    dut1.out1.dat(out_dat);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }


  void stim() {
    CCS_LOG("Stimulus started");
    in_vld = 0;
    in_dat = 0;
    wait();

    stable_random gen;

    for (int i = 0; i < 10; i++) {
      in_vld = 1;
      in_dat = i;
      wait();
      in_vld = 0;
      in_dat = 0;
      wait();
      if (gen.get() & 1)
        wait();
    }

    wait(50);
    sc_stop();
    wait();
  }

  void resp() {
    wait();

    while (1) {
      wait();
      if (out_vld)
        CCS_LOG("TB resp sees: " << std::hex << out_dat);
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  auto top = std::make_shared<Top>("top");
  trace_hierarchy(top.get(), trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(*top);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


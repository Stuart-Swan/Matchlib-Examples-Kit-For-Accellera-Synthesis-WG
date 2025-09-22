// INSERT_EULA_COPYRIGHT: 2020

#include <ac_sysc_trace.h>
#include "dut.h"
#include <memory>

#include <mc_scverify.h>
#include <stable_random.h>

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);
  SC_SIG(bool, is_full);
  SC_SIG(bool, is_empty);
  SC_SIG(STATUS_TYPE, num_free);
  SC_SIG(STATUS_TYPE, num_filled);

  Connections::Combinational<uint32>        SC_NAMED(out1);
  Connections::Combinational<uint32>        SC_NAMED(in1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);
    dut1.is_full(is_full);
    dut1.is_empty(is_empty);
    dut1.num_free(num_free);
    dut1.num_filled(num_filled);

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
    in1.ResetWrite();
    wait();

    stable_random gen;

    for (int i = 0; i < 40; i++) {
      while (gen.get() & 1)
        wait();
      in1.Push(i);
    }

    wait();
    wait();
    wait();
    wait();
    wait();
    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    stable_random gen;

    while (1) {
      while (gen.get() & 1)
        wait();
      CCS_LOG("TB resp sees: " << std::hex << out1.Pop());
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
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


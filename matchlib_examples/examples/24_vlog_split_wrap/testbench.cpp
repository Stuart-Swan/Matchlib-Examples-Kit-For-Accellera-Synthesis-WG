// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>
#include <memory.h>

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<transaction_t>        SC_NAMED(out1);
  Connections::Combinational<transaction_t>        SC_NAMED(in1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

#ifndef CCS_SYSC
    auto_gen_split_wrap wrap1("dut");
    dut1.gen_port_info_vec(wrap1.port_info_vec);
    wrap1.gen_wrapper();
#endif

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);

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

    for (int i = 0; i < 10; i++) {
      transaction_t t;
      t.value = i;
      in1.Push(t);
    }

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      transaction_t t = out1.Pop();
      CCS_LOG("TB resp sees: " << std::hex << t.value);
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


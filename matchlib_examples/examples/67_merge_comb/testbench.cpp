// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>
#include <stable_random.h>
#include <memory>

#include "dut.h"

class Top : public sc_module
{
public:
  CCS_DESIGN(merge_comb) SC_NAMED(merge_comb1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<type1_t>       SC_NAMED(in1);
  Connections::Combinational<type2_t>       SC_NAMED(in2);
  Connections::Combinational<type3_t>       SC_NAMED(in3);
  Connections::Combinational<out_t>       SC_NAMED(out1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    merge_comb1.clk(clk);
    merge_comb1.rst_bar(rst_bar);
    merge_comb1.out1(out1);
    merge_comb1.in1(in1);
    merge_comb1.in2(in2);
    merge_comb1.in3(in3);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim1);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(stim2);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(stim3);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }


  void stim1() {
    CCS_LOG("Stimulus1 started");
    in1.ResetWrite();
    wait();

    stable_random gen;

    for (int i = 0; i < 30; ) {
      wait();
      if (gen.get() & 1) {
        in1.Push(i);
        ++i;
      }
    }

    sc_stop();
    wait();
  }

  void stim2() {
    CCS_LOG("Stimulus2 started");
    in2.ResetWrite();
    wait();

    stable_random gen;

    for (int i = 0x0; i < 30; ) {
      wait();
      if (gen.get() & 1) {
        in2.Push(i + 0x100);
        ++i;
      }
    }

    sc_stop();
    wait();
  }

  void stim3() {
    CCS_LOG("Stimulus3 started");
    in3.ResetWrite();
    wait();

    stable_random gen;

    for (int i = 0; i < 30; ) {
      wait();
      if (gen.get() & 1) {
        in3.Push(i + 0x200);
        ++i;
      }
    }

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();
    stable_random gen;

    while (1) {
      wait();
      if (gen.get() & 1) {
        out_t pkt = out1.Pop();
        CCS_LOG("TB resp sees: " << std::hex << pkt );
      }
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


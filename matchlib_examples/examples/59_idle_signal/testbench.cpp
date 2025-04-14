// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>
#include <stable_random.h>
#include <memory.h>

#define USE_GATED_CLOCK 1
#define SAMPLE_COUNT  300
#define USE_RANDOM 1
//#define AUTOMATIC_IDLE 1

#include "dut.h"
#include "clock_gate_module.h"


class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);
  SC_SIG(bool, idle);
  SC_SIG(bool, gated_clk);
  clock_gate_module SC_NAMED(clock_gate_module1);

  Connections::Combinational<uint32>        SC_NAMED(out1);
  Connections::Combinational<uint32>        SC_NAMED(in1);
  Connections::Combinational<uint32>        SC_NAMED(in2);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true)
  {
    sc_object_tracer<sc_clock> trace_clk(clk);

    clock_gate_module1.idle(idle);
    clock_gate_module1.clk_in(clk);
    clock_gate_module1.clk_out(gated_clk);

#ifdef USE_GATED_CLOCK
    dut1.clk(gated_clk);
#else
    dut1.clk(clk);
#endif
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);
    dut1.in2(in2);
    dut1.idle(idle);

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
    in2.ResetWrite();
    wait();

    stable_random gen;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
      in1.Push(i);
      in2.Push(i);
      if ((i % 10) == 0)
      {
#ifdef USE_RANDOM
        uint32 t = gen.get() & 0xf;
        for (int i=0; i < t; i++)
          wait();
#else
        wait(8);
#endif
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
      auto t = out1.Pop();
      CCS_LOG("TB resp sees: " << std::hex << t);
#ifdef USE_RANDOM
      if ((gen.get() & 0xf) == 0)
        wait();
#endif
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
    wait(SAMPLE_COUNT * 8);
    CCS_LOG("stopping due to timeout");
    sc_stop();
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
#ifdef USE_GATED_CLOCK
  logs.enable("gated_clock_log");
#else
  logs.enable("regular_clock_log");
#endif
  logs.log_hierarchy(*top);

  sc_start();

  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


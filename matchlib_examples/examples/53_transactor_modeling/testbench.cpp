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

  hilo_chan SC_NAMED(in1_hilo_chan); // hilo protocol channel for input to DUT
  hilo_chan SC_NAMED(in2_hilo_chan); // hilo protocol channel for input to DUT
  hilo_chan SC_NAMED(out1_hilo_chan); // hilo protocol channel for output from DUT

#ifdef USE_THREAD
  hilo_in_xact<sc_uint<16>> SC_NAMED(out1_xact); // IN xactor converts out1_hilo_chan to Connections
  hilo_out_xact<sc_uint<16>> SC_NAMED(in1_xact); // OUT xactor converts Connections to in1_hilo_chan
  hilo_out_xact<sc_uint<16>> SC_NAMED(in2_xact);
#endif

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1(in1_hilo_chan);
    dut1.in2(in2_hilo_chan);
    dut1.out1(out1_hilo_chan);

#ifdef USE_THREAD
    out1_xact.bind(clk, rst_bar, out1_hilo_chan);
    in1_xact.bind(clk, rst_bar, in1_hilo_chan);
    in2_xact.bind(clk, rst_bar, in2_hilo_chan);
#endif

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
#ifdef USE_THREAD
    in1_xact.chan.ResetWrite();
    in2_xact.chan.ResetWrite();
#else
    hilo_reset_write(in1_hilo_chan);
    hilo_reset_write(in2_hilo_chan);
#endif
    wait();

#ifdef REORDER_STIM
    bool reorder=1;
#else
    bool reorder=0;
#endif

    for (int i = 1; i < 20; i++) {
      if (!reorder || (i != 7)) {
#ifdef USE_THREAD
        in1_xact.chan.Push(i);
        in2_xact.chan.Push(i);
#else
        hilo_push(in1_hilo_chan, i);
        hilo_push(in2_hilo_chan, i);
#endif
      } else {
#ifdef USE_THREAD
        in2_xact.chan.Push(i);
        in1_xact.chan.Push(i);
#else
        hilo_push(in2_hilo_chan, i);
        hilo_push(in1_hilo_chan, i);
#endif
      }
    }
  }

  void resp() {
#ifdef USE_THREAD
    out1_xact.chan.ResetRead();
#else
    hilo_reset_read(out1_hilo_chan);
#endif
    wait();

    int cnt=0;

    while (1) {
#ifdef USE_THREAD
      auto o1 = out1_xact.chan.Pop();
#else
      auto o1 = hilo_pop(out1_hilo_chan);
#endif
      CCS_LOG("See: " << o1);
      if (++cnt > 15) {
        sc_stop();
        wait();
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
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


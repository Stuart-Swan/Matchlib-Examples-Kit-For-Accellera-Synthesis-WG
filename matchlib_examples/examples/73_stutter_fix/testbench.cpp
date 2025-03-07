// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>

#include "dut.h"
#include "ram.h"
#include <memory.h>


class Top : public sc_module, public local_axi
{
public:
  CCS_DESIGN(dut)                         SC_NAMED(dut1);   // the DUT
  ram                                     SC_NAMED(ram1);
  sc_clock                                clk;
  sc_signal<bool>                         rst_bar;

  Connections::Combinational<uint32>      SC_NAMED(in1_chan);
  Connections::Combinational<uint32>      SC_NAMED(in2_chan);
  Connections::Combinational<uint32>      SC_NAMED(in3_chan);
  Connections::Combinational<uint32>      SC_NAMED(out1_chan);
  w_chan<> SC_NAMED(dut_w_master);
  r_chan<> SC_NAMED(dut_r_master);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1(in1_chan);
    dut1.in2(in2_chan);
    dut1.in3(in3_chan);
    dut1.out1(out1_chan);
    dut1.r_master0(dut_r_master);
    dut1.w_master0(dut_w_master);

    ram1.clk(clk);
    ram1.rst_bar(rst_bar);
    ram1.r_slave0(dut_r_master);
    ram1.w_slave0(dut_w_master);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    sc_object_tracer<sc_clock> trace_clk(clk);
  }

  sc_time start_time, end_time;


  void stim() {
    CCS_LOG("Stimulus started");
    in1_chan.ResetWrite();
    in2_chan.ResetWrite();
    in3_chan.ResetWrite();
    wait();

    start_time = sc_time_stamp();

    // First write to the first N locations of the memory in DUT
    for (int i=0; i < 10; i++) {
#ifdef SKIP_INITIALIZATION
      if (i == 5)
        continue;
#endif

      in1_chan.Push(0);
      in2_chan.Push(i);
      in3_chan.Push(i * 8);
    }

    // Now read from the first N locations of the memory in DUT
    for (int i=0; i < 10; i++) {
      in1_chan.Push(i + 1);
      in2_chan.Push(i);
      in3_chan.Push(i * 8);
    }

    wait(20, SC_NS);
    sc_stop();
    wait();
  }

  void resp() {
    out1_chan.ResetRead();
    wait();

    while (1) {
      uint32 r = out1_chan.Pop();
      CCS_LOG("resp: " << r);
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
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  auto top = std::make_shared<Top>("top");
  trace_hierarchy(top.get(), trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(*top);

  sc_start();
  return 0;
}


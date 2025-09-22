// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>

#include "dut.h"
#include <memory>


class Top : public sc_module
{
public:
  CCS_DESIGN(dut)                         SC_NAMED(dut1);   // the DUT
  sc_clock                                clk;
  sc_signal<bool>                         rst_bar;

  Connections::Combinational<uint32>      SC_NAMED(mask_chan);
  Connections::Combinational<uint32>      SC_NAMED(data_chan);
  Connections::Combinational<uint32>      SC_NAMED(addr_chan);
  Connections::Combinational<uint32>      SC_NAMED(out1_chan);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.mask_port(mask_chan);
    dut1.data_port(data_chan);
    dut1.addr_port(addr_chan);
    dut1.out1(out1_chan);

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
    mask_chan.ResetWrite();
    data_chan.ResetWrite();
    addr_chan.ResetWrite();
    wait();

    start_time = sc_time_stamp();
    // initialize mem so it doesn't have X values:
    for (int i=0; i < 16; i++) {
      mask_chan.Push(~0);
      data_chan.Push(0);
      addr_chan.Push(i * 8);
    }

    // First write to the first N locations of the memory in DUT
    for (int i=0; i < 16; i++) {
      mask_chan.Push(i + 1);
      data_chan.Push((i << 24) | (i << 16) | (i << 8) | i);
      addr_chan.Push(i * 8);
    }

    // Now read from the first N locations of the memory in DUT
    for (int i=0; i < 16; i++) {
      mask_chan.Push(0);
      data_chan.Push(0);
      addr_chan.Push(i * 8);
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
      CCS_LOG("resp: " << std::hex << r);
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


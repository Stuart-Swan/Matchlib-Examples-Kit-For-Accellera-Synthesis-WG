// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>
#include <stable_random.h>
#include <memory.h>

#include "dut.h"

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<packet>       SC_NAMED(in1);
  Connections::Combinational<packet>       SC_NAMED(in2);
  Connections::Combinational<packet>       SC_NAMED(out1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);
    dut1.in2(in2);

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

    packet pkt;
    stable_random gen;

    for (int i = 0; i < 30; ) {
      wait();
      if (gen.get() & 1) {
        for (int z=0; z < packet::SIZE; z++) 
          pkt.data[z] = i;
        in1.Push(pkt);
        ++i;
      }
      if (gen.get() & 1) {
        for (int z=0; z < packet::SIZE; z++) 
          pkt.data[z] = i;
        in2.Push(pkt);
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
        packet pkt = out1.Pop();
        uint32 t = pkt.data[0];
        CCS_LOG("TB resp sees: " << std::hex << t);
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


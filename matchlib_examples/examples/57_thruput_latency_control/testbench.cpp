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

  Connections::Combinational<packet>        SC_NAMED(out1);
  Connections::Combinational<packet>        SC_NAMED(in1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

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

    packet p;

    for (int i=0; i < packet::data_len; i++) {
      p.data[i] = i;
    }

    for (int i = 0; i < num_packets; i++) {
      if (i & 0x3) {
        if ((i & 0x3) == 1) {
          p.coeff = 1;
        } else {
          p.coeff = 0;
        }
      } else {
        p.coeff = i;
      }

      in1.Push(p);
    }
  }

  void resp() {
    out1.ResetRead();
    wait();

    sc_time start_time = sc_time_stamp();

    for (int i = 0; i < num_packets; i++) {
      packet p = out1.Pop();
      CCS_LOG("TB resp sees: " << std::hex << p.coeff << " " << p.data[1]);
    }

    sc_time end_time = sc_time_stamp();
    CCS_LOG("Throughput is: " << (end_time - start_time) / num_packets);

    sc_stop();
    wait();
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
  }

  const int num_packets = 50;
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


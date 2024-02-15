// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) CCS_INIT_S1(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<sc_uint<32>>        CCS_INIT_S1(in1);
  Connections::Combinational<sc_uint<32>>        CCS_INIT_S1(in2);
  Connections::Combinational<sc_uint<32>>        CCS_INIT_S1(in3);
  Connections::Combinational<sc_uint<32>>        CCS_INIT_S1(out1);
  Connections::Combinational<sc_uint<32>>        CCS_INIT_S1(out2);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1(in1);
    dut1.in2(in2);
    dut1.in3(in3);
    dut1.out1(out1);
    dut1.out2(out2);

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
    in3.ResetWrite();
    wait();

    for (int i = 0; i < 10; i++) {
      in1.Push(i);
      if (i & 1) {
        in3.Push(0x1000);
        in2.Push(0x100);
      } else {
        in2.Push(0x0);
        in3.Push(0x0);
      }
    }

    wait();
  }

  void resp() {
    out1.ResetRead();
    out2.ResetRead();
    wait();

    for (int i = 0; i < 10; i++) {
      CCS_LOG("TB resp sees: " << std::hex << out1.Pop() << " " << out2.Pop());
    }

    sc_stop();
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

  Top top("top");
  trace_hierarchy(&top, trace_file_ptr);
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


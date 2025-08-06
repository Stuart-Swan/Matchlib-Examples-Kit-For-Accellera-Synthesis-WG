// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>
#include <memory.h>
#include <stable_random.h>

class Top : public sc_module {
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);
  sc_clock clk;
  SC_SIG(bool, rst_bar);
  Connections::Combinational<ac_int<32, false>>        SC_NAMED(out1);
  Connections::Combinational<ac_int<32, false>>        SC_NAMED(in1);

  SC_CTOR(Top) 
     :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) 
  {
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

    stable_random gen;

    for (int i = 0; i < 1000000; i++) {
      if (gen.get() & 1) {
        wait();
        wait();
        wait();
      }
      in1.Push(i);
    }

    wait(1000);
    
    sc_stop();
  }

  void resp() {
    out1.ResetRead();
    wait();
    stable_random gen;
    int cnt=0;
    int mismatches=0;

    for (int i = 0; i < 1000000; i++) {
      if (gen.get() & 1) {
        wait();
        wait();
        wait();
      }
      int t = out1.Pop();
      ++cnt;
      if (t != i) {
        CCS_LOG("mismatch at count: " << std::dec << cnt << " " << std::hex << t << " " << i);
        ++mismatches;
        if (mismatches > 20)
          sc_stop();
      }
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


sc_trace_file* trace_file_ptr;

int sc_main(int argc, char** argv) {
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  // trace_file_ptr = sc_create_vcd_trace_file("trace");

  auto top = std::make_shared<Top>("top");
  // trace_hierarchy(top.get(), trace_file_ptr);
  sc_start();
  return 0;
}

// INSERT_EULA_COPYRIGHT: 2020

#include <memory>

#include "new_annotate.h"

#include "reconvergence.h"

class Testbench : public sc_module
{
public:

  top SC_NAMED(dut1);

  sc_clock clk;
  sc_signal<bool> SC_NAMED(rstn);

  Connections::Combinational<sc_uint<16>>        SC_NAMED(din);
  Connections::Combinational<sc_uint<16>>        SC_NAMED(dout);

  SC_CTOR(Testbench)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    dut1.clk(clk);
    dut1.rstn(rstn);
    dut1.din(din);
    dut1.dout(dout);

    SC_THREAD(reset);
    sensitive << clk.posedge_event();

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);
  }

  // Drives "din" Matchlib port on DUT
  void stim() {
    std::cout << "Stimulus reset started\n";
    din.ResetWrite();
    wait();

    for (int i = 1; i < 20; i++) {
      din.Push(i);
      std::cout << "STIM Pushed: " << std::hex << i << "\n";
    }
    wait(100);
  }

  void resp() {
    dout.ResetRead();
    wait();
    for (int i = 1; i < 20; i++) {
      sc_uint<16> res = dout.Pop();
      std::cout << "RESP Popped: " << std::hex << res << "\n";
    }
    sc_stop();
  }

  void reset() {
    rstn.write(0);
    wait(3); // Drive reset for 3 clock cycles
    rstn.write(1);
    wait();
  }

};

int sc_main(int argc, char **argv)
{
  // Configure error handling
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  // Create a trace file

  auto testbench = std::make_shared<Testbench>("testbench");

  // Trace waveforms
  //trace_hierarchy(testbench.get(), trace_file_ptr);

  // Enable data logging
  channel_logs logs;
  logs.enable("chan_log",true);
  logs.log_hierarchy(*testbench);

  Connections::annotate annot;
  annot.output_file(*testbench, "testbench.output.json");
  annot.input_file(*testbench, "testbench.input.json", 1);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED with " << sc_report_handler::get_count(SC_ERROR) << " errors" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}

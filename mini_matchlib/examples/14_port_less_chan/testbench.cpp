
#include "dut.h"
#include <memory>

class Top : public sc_module
{
public:
  dut SC_NAMED(dut1);

  sc_clock clk;
  sc_signal<bool> SC_NAMED(rst_bar);

  Connections::Combinational<uint32_t>        SC_NAMED(out1);
  Connections::Combinational<uint32_t>        SC_NAMED(in1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    Connections::set_sim_clk(&clk);

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
    std::cout << "Stimulus started\n";
    in1.ResetWrite();
    wait();

    for (int i = 0; i < 10; i++) {
      in1.Push(i);
    }

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      auto v = out1.Pop();
      std::cout << "TB resp sees: " << std::hex << v << "\n";
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

  auto top = std::make_shared<Top>("top");

  sc_start();

  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


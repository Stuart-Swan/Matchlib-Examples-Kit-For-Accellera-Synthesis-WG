
#include "dut.h"
#include <memory>

class Top : public sc_module
{
public:
  dut SC_NAMED(dut1);

  sc_clock clk;
  sc_signal<bool> SC_NAMED(rst_bar);

  sc_signal<bool> SC_NAMED(in_vld);
  sc_signal<bool> SC_NAMED(out_vld);
  sc_signal<unsigned> SC_NAMED(out_dat);
  sc_signal<unsigned> SC_NAMED(in_dat);

  SC_CTOR(Top)
    :   clk ("clk",  1, SC_NS, 0.5,0,SC_NS,true) 
  {
    Connections::set_sim_clk(&clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1.vld(in_vld);
    dut1.in1.dat(in_dat);
    dut1.out1.vld(out_vld);
    dut1.out1.dat(out_dat);

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
    in_vld = 0;
    in_dat = 0;
    wait();

    for (int i = 0; i < 10; i++) {
      in_vld = 1;
      in_dat = i;
      wait();
      in_vld = 0;
      in_dat = 0;
      wait();
      if (rand() & 1)
        wait();
    }

    wait(50);
    sc_stop();
    wait();
  }

  void resp() {
    wait();

    while (1) {
      wait();
      if (out_vld) 
        std::cout << sc_time_stamp() << " TB resp sees: " << std::hex << out_dat << "\n";
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


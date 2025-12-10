
#include "dut.h"
#include <memory>

class testbench : public sc_module
{
public:
  dut SC_NAMED(dut1);

  sc_clock clk;
  sc_signal<bool> SC_NAMED(rst_bar);

  Connections::Combinational<uint32_t>        SC_NAMED(out1);
  sc_vector<Connections::Combinational<uint32_t>> sample_in{"sample_in", num_samples};
  //Connections::Combinational<uint32_t>        sample_in[num_samples];
  Connections::SyncChannel                    SC_NAMED(sync_chan);
  sc_signal<uint32_t>                         direct_inputs[num_direct_inputs];

  SC_HAS_PROCESS(testbench);
  testbench(const sc_module_name& name)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    Connections::set_sim_clk(&clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.sync_in(sync_chan);

    for (int i=0; i < num_samples; i++) {
      dut1.sample_in[i](sample_in[i]);
    }

    for (int y=0; y < num_direct_inputs; y++) {
      dut1.direct_inputs[y](direct_inputs[y]);
    }

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
    sync_chan.ResetWrite();

    for (int i=0; i < num_samples; i++) {
      sample_in[i].ResetWrite();
    }

    for (int y=0; y < num_direct_inputs; y++) {
      direct_inputs[y] = y;
    }

    direct_inputs[0] = 5;
    direct_inputs[1] = 5;
    direct_inputs[7] = 4;

    wait();


    for (int i = 0; i < 10; i++) {
      do {
        wait();
      } while (sync_chan.rdy == 0);

      // update dir inputs here
      direct_inputs[2] = 1 + direct_inputs[2];
      direct_inputs[3] = direct_inputs[4];
      direct_inputs[4] = direct_inputs[3];
      direct_inputs[7] = 4 + (i&3);
      sync_chan.vld = 1;

      wait();
      sync_chan.vld = 0;

      if (i == 9) {
        wait(100);
        sc_stop();
        wait();
      }

      for (uint32_t x=0; x < direct_inputs[0]; x++) {
        for (uint32_t y=0; y < direct_inputs[1]; y++) {
          for (uint32_t s=0; s < num_samples; s++) {
            sample_in[s].Push((x*y) + i + s);
          }
        }
      }
    }

    wait(50);

    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      uint32_t t = out1.Pop();
      std::cout << "TB resp sees: " << std::hex << t  << "\n";
    }
  }

  void end_of_simulation() {}

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

  auto top = std::make_shared<testbench>("top");
  sc_start();
  return 0;
}


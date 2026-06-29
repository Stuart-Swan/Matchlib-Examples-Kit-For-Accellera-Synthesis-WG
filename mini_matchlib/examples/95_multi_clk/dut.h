
#pragma once
#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk5);
  sc_in<bool> SC_NAMED(rst_bar);

  msg_out<uint32_t> SC_NAMED(out1);
  msg_in <uint32_t> SC_NAMED(in1);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk5.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1.reset_push();
    in1.reset_pop();

    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      uint32_t t = in1.pop();
      out1.push(t + 0x100);
    }
  }
};

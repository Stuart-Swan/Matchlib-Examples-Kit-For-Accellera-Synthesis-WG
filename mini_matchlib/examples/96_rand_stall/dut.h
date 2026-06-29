
#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  msg_in <sc_uint<32>> SC_NAMED(in1);
  msg_in <sc_uint<32>> SC_NAMED(in2);
  msg_in <sc_uint<32>> SC_NAMED(in3);
  msg_out<sc_uint<32>> SC_NAMED(out1);
  msg_out<sc_uint<32>> SC_NAMED(out2);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    in1.reset_pop();
    in2.reset_pop();
    in3.reset_pop();
    out1.reset_push();
    out2.reset_push();
    wait();     
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<32> i1 = in1.pop();
      sc_uint<32> i2 = in2.pop();
      sc_uint<32> i3 = in3.pop();
      out1.push(i1 + i2);
      out2.push(i1 + i3);
    }
  }
};


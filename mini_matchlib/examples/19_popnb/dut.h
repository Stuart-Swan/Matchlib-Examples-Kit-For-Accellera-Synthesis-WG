// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <msg_lib.h>

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  msg_lib::msg_in <sc_uint<32>> SC_NAMED(in1);
  msg_lib::msg_in <sc_uint<32>> SC_NAMED(in2);
  msg_lib::msg_out<sc_uint<32>> SC_NAMED(out1);
  msg_lib::msg_out<sc_uint<32>> SC_NAMED(out2);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    in1.Reset();
    in2.Reset();
    out1.Reset();
    out2.Reset();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<32> i1; 
      sc_uint<32> i2;
      bool b1, b2;

      b1 = in1.PopNB(i1);
      b2 = in2.PopNB(i2);
      if (b1)
        out1.Push(i1);
      if (b2)
        out2.Push(i2);

      wait();
    }
  }
};

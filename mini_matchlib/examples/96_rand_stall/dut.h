
#pragma once

#include <new_connections.h>

class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In <sc_uint<32>> SC_NAMED(in1);
  Connections::In <sc_uint<32>> SC_NAMED(in2);
  Connections::In <sc_uint<32>> SC_NAMED(in3);
  Connections::Out<sc_uint<32>> SC_NAMED(out1);
  Connections::Out<sc_uint<32>> SC_NAMED(out2);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    in1.Reset();
    in2.Reset();
    in3.Reset();
    out1.Reset();
    out2.Reset();
    wait();     
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<32> i1 = in1.Pop();
      sc_uint<32> i2 = in2.Pop();
      sc_uint<32> i3 = in3.Pop();
      out1.Push(i1 + i2);
      out2.Push(i1 + i3);
    }
  }
};


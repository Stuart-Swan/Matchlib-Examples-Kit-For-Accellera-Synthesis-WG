// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#pragma hls_design top
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
      uint32_t i1 = in1.Pop();
      uint32_t i2 = in2.Pop();
      uint32_t i3 = in3.Pop();
      out1.Push(i1 + i2);
      out2.Push(i1 + i3);
    }
  }

  void main() {
    in1.Reset();
    in2.Reset();
    in3.Reset();
    out1.Reset();
    out2.Reset();
    wait();

    bool got_i1{0}, got_i2{0}, got_i3{0};
    bool got_o1{0}, got_o2{0};
    sc_uint<32> i1, i2, i3, o1, o2;

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      wait();

      if (!got_i1) got_i1 = in1.PopNB(i1);
      if (!got_i2) got_i2 = in1.PopNB(i2);
      if (!got_i3) got_i3 = in1.PopNB(i3);

      if (got_i1 && got_i2 && !got_o1) {
        o1 = i1 + i2;
        got_o1 = 1;
      }

      if (got_i1 && got_i3 && !got_o2) {
        o2 = i1 + i3;
        got_o2 = 1;
      }

      if (got_o1 && got_o2) {
        got_i1 = 0;
        got_i2 = 0;
        got_i3 = 0;
      }

      if (got_o1)
       if (out1.PushNB(o1))
         got_o1 = 0;

      if (got_o2)
       if (out2.PushNB(o2))
         got_o2 = 0;
    }
  }
};


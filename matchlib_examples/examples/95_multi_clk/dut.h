// INSERT_EULA_COPYRIGHT: 2020

#pragma once
#include <mc_connections.h>

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk5);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<uint32> SC_NAMED(out1);
  Connections::In <uint32> SC_NAMED(in1);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk5.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1.Reset();
    in1.Reset();

    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      uint32_t t = in1.Pop();
      out1.Push(t + 0x100);
    }
  }
};


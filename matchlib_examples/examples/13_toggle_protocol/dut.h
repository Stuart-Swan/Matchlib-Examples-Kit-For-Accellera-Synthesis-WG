// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "sc_named.h"

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  sc_out<sc_uint<32>>  SC_NAMED(out1_data);
  sc_out<bool>         SC_NAMED(out1_toggle);

  Connections::In <sc_uint<32>> SC_NAMED(in1);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1_data = 0;
    out1_toggle = false;
    in1.Reset();
    bool toggle = false;
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      uint32_t t = in1.Pop();
      out1_data = t;
      toggle = !toggle;
      out1_toggle = toggle;
      wait();
    }
  }
};


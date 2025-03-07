// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#pragma hls_design top
class flop : public sc_module
{
public:
  sc_in<bool>      SC_NAMED(clk);
  sc_in<bool>      SC_NAMED(rst_bar);
  sc_in<uint32_t>  SC_NAMED(in1);
  sc_out<uint32_t> SC_NAMED(out1);

  SC_CTOR(flop) {
    SC_THREAD(process);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void process() {
    // this is the reset state:
    out1 = 0;
    wait();                                 // WAIT
    // this is the non-reset state:
    while (1) {
      out1 = in1.read();
      wait();                              // WAIT
    }
  }
};


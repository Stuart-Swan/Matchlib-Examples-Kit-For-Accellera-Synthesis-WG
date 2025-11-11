// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <ac_sysc_macros.h>
#include <mc_connections.h>

#include "sc_named.h"

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

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool>      SC_NAMED(clk);
  sc_in<bool>      SC_NAMED(rst_bar);
  sc_in<uint32_t>  SC_NAMED(in1);
  sc_out<uint32_t> SC_NAMED(out1);

  const unsigned len = 5;
  sc_vector<flop> flop_array{"flop_array", len};
  sc_vector<sc_signal<uint32_t>> sig_array{"sig_array", len};

  SC_CTOR(dut) {

    for (unsigned i=0; i < len; i++) {
      flop_array[i].clk(clk);
      flop_array[i].rst_bar(rst_bar);

      if (i == 0) {
        flop_array[i].in1(in1);
      } else {
        flop_array[i].in1(sig_array[i-1]);
      }

      if (i == (len-1)) {
        flop_array[i].out1(out1);
      } else {
        flop_array[i].out1(sig_array[i]);
      }
    }
  }
};

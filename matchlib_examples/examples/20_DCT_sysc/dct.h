// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <ac_int.h>
#include <ac_fixed.h>
#include <mc_connections.h>
#include "RAM_1R1W.h"

SC_MODULE(dct)
{
  public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rstn);

  Connections::In<ac_int<8> > SC_NAMED(input);
  RAM_1R1W_model<>::wr1_port<ac_int<16>,128> SC_NAMED(output); // Ping-pong output memory
  Connections::SyncOut SC_NAMED(sync_out);

  SC_CTOR(dct) {
    SC_THREAD(dct_h);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    SC_THREAD(dct_v);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    // Instantiated memories must bind clocks
    mem.CK(clk);
  }

  void dct_h();
  void dct_v();

  private:
  Connections::SyncChannel SC_NAMED(valid); // memory synchronization between threads
  RAM_1R1W_model<>::mem<ac_int<16>,128> SC_NAMED(mem); // Ping-pong shared memory
};


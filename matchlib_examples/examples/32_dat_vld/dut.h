// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "dat_vld.h"

#pragma hls_design top
class dut : public sc_module
{
public:
  typedef uint32 T;
  sc_in<bool>    SC_NAMED(clk);
  sc_in<bool>    SC_NAMED(rst_bar);

  dat_vld_in<T>   SC_NAMED(in1);
  dat_vld_out<T>  SC_NAMED(out1);

  dat_vld_in_xact<T>  SC_NAMED(in1_xact);
  dat_vld_out_xact<T> SC_NAMED(out1_xact);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    in1_xact.bind(in1);
    out1_xact.bind(out1);
  }

private:

  void main() {
    out1_xact.chan.ResetWrite();
    in1_xact.chan.ResetRead();
    wait(); 
// NOTE: stall_mode must be stall for this example. If it is flush then FSM may apply backpressure,
// which would cause the protocol adaptor to operate incorrectly

#pragma hls_pipeline_init_interval 2
#pragma pipeline_stall_mode stall
    while (1) {
      uint32 x = in1_xact.chan.Pop();
      uint32 y = 0;
      y += 3 * x;
      y += 7 * x * x;
      out1_xact.chan.Push(y);
    }
  }
};

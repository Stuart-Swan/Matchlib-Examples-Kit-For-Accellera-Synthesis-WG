
#pragma once

#include "dat_vld.h"

#pragma hls_design top
class dut : public sc_module
{
public:
  typedef unsigned T;
  sc_in<bool>    SC_NAMED(clk);
  sc_in<bool>    SC_NAMED(rst_bar);

  dat_vld_in_xact<T>   SC_NAMED(in1);
  dat_vld_out_xact<T>  SC_NAMED(out1);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1.chan.ResetWrite();
    in1.chan.ResetRead();
    wait();
// NOTE: stall_mode must be stall for this example. If it is flush then FSM may apply backpressure,
// which would cause the protocol adaptor to operate incorrectly

#pragma hls_pipeline_init_interval 2
#pragma pipeline_stall_mode stall
    while (1) {
      unsigned x = in1.chan.Pop();
      unsigned y = 0;
      y += 3 * x;
      y += 7 * x * x;
      out1.chan.Push(y);
    }
  }
};


#pragma once

#include <msg_lib.h>
#include "connections_fifo.h"


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  typedef sc_uint<32> T;
  static const int N = 8;

  msg_lib::msg_in <T> SC_NAMED(in1);
  msg_lib::msg_out<T> SC_NAMED(out1);

  msg_lib::Fifo<T, N> SC_NAMED(fifo1);
  msg_lib::msg_chan<T> SC_NAMED(fifo1_in1);
  msg_lib::msg_chan<T> SC_NAMED(fifo1_out1);

  SC_CTOR(dut) {
    fifo1.clk(clk);
    fifo1.rst(rst_bar);
    fifo1.enq(fifo1_in1);
    fifo1.deq(fifo1_out1);

    SC_THREAD(main1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(main2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main1() {
    fifo1_in1.ResetWrite();
    in1.Reset();
    wait();  
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { fifo1_in1.Push(in1.Pop()); }
  }

  void main2() {
    fifo1_out1.ResetRead();
    out1.Reset();
    wait();         
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { out1.Push(fifo1_out1.Pop()); }
  }
};

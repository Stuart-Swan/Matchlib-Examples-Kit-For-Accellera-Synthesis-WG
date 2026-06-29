
#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;
#include "sc_hls_fifo.h"


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  typedef sc_uint<32> T;
  static const int N = 8;

  msg_in <T> SC_NAMED(in1);
  msg_out<T> SC_NAMED(out1);

  msg_fifo<T, N> SC_NAMED(fifo1);
  msg_channel<T> SC_NAMED(fifo1_in1);
  msg_channel<T> SC_NAMED(fifo1_out1);

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
    fifo1_in1.reset_push();
    in1.reset_pop();
    wait();  
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { fifo1_in1.push(in1.pop()); }
  }

  void main2() {
    fifo1_out1.reset_pop();
    out1.reset_push();
    wait();         
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { out1.push(fifo1_out1.pop()); }
  }
};

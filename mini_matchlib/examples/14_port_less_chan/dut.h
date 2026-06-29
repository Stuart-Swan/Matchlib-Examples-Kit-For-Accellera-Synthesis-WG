
#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  msg_in <uint32_t> SC_NAMED(in1);
  msg_out<uint32_t> SC_NAMED(out1);

  msg_channel<uint32_t> SC_NAMED(chan1);

  SC_CTOR(dut) {
    SC_THREAD(main1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(main2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main1() {
    in1.reset_pop();
    chan1.reset_push();
    wait();  
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { 
      chan1.push(in1.pop()); 
    }
  }

  void main2() {
    chan1.reset_pop();
    out1.reset_push();
    wait();                           
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { 
      out1.push(chan1.pop()); 
    }
  }
};


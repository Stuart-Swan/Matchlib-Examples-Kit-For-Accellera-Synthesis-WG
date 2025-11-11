
#pragma once

#include "../include/new_connections.h"


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In <uint32_t> SC_NAMED(in1);
  Connections::Out<uint32_t> SC_NAMED(out1);

  Connections::Combinational<uint32_t> SC_NAMED(chan1);

  SC_CTOR(dut) {
    SC_THREAD(main1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(main2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main1() {
    in1.Reset();
    chan1.ResetWrite();
    wait();  
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { 
      chan1.Push(in1.Pop()); 
    }
  }

  void main2() {
    chan1.ResetRead();
    out1.Reset();
    wait();                           
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { 
      out1.Push(chan1.Pop()); 
    }
  }
};


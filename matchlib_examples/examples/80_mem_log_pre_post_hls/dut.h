// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "extended_array.h"


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);
  sc_in<bool> SC_NAMED(zero);

  typedef ac_int<32, false> elem_type;
  static const int mem_size = 128;


  Connections::In <elem_type> SC_NAMED(raddr_in);
  Connections::In <elem_type> SC_NAMED(waddr_in);
  Connections::In <elem_type> SC_NAMED(data_in);
  Connections::Out<elem_type> SC_NAMED(data_out);

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }
#ifdef USE_EXT_ARRAY
  extended_array<elem_type, mem_size> mem{"mem_prehls"};
#endif

  void main() {
#ifndef USE_EXT_ARRAY
    elem_type mem[mem_size];
#endif

    raddr_in.Reset();
    waddr_in.Reset();
    data_in.Reset();
    data_out.Reset();

    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode stall
    for (unsigned u=0; u < mem_size; u++)
      mem[u] = zero.read();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      mem[waddr_in.Pop()] = data_in.Pop();
      elem_type t = mem[raddr_in.Pop()];
      data_out.Push(t);
    }
  }
};

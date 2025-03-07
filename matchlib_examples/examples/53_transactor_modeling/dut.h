// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "hilo.h"

#define USE_THREAD 1
//#define USE_MIO 1
//#define REORDER_STIM 1


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  hilo_in SC_NAMED(in1);
  hilo_in SC_NAMED(in2);
  hilo_out SC_NAMED(out1);

#ifdef USE_THREAD
  hilo_in_xact<sc_uint<16>> SC_NAMED(in1_xact);
  hilo_in_xact<sc_uint<16>> SC_NAMED(in2_xact);
  hilo_out_xact<sc_uint<16>> SC_NAMED(out1_xact);
#endif

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifdef USE_THREAD
    in1_xact.bind(clk, rst_bar, in1);
    in2_xact.bind(clk, rst_bar, in2);
    out1_xact.bind(clk, rst_bar, out1);
#endif
  }

private:

  void main() {
#ifdef USE_THREAD
    in1_xact.chan.ResetRead();
    in2_xact.chan.ResetRead();
    out1_xact.chan.ResetWrite();
#else
    hilo_reset_read(in1);
    hilo_reset_read(in2);
    hilo_reset_write(out1);
#endif
    wait();

// simple function based transactors cannot be pipelined, but USE_MIO and USE_THREAD can be

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
#ifdef USE_THREAD
      auto i1 = in1_xact.chan.Pop();
      auto i2 = in2_xact.chan.Pop();
      out1_xact.chan.Push(i1 + i2);
#else
#ifdef USE_MIO
      auto i1 = hilo_pop_mio(in1);
      auto i2 = hilo_pop_mio(in2);
      hilo_push_mio(out1, i1 + i2);
#else
      auto i1 = hilo_pop(in1);
      auto i2 = hilo_pop(in2);
      hilo_push(out1, i1 + i2);
#endif
#endif
    }
  }
};

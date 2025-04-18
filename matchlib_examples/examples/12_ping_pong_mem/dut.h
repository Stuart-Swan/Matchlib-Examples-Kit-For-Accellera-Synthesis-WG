// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include <ac_shared_array_1D.h>
#include <extended_array.h>

#include <sc_named.h>

SC_MODULE(dut)
{
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In<ac_int<16> >  SC_NAMED(in1);
  Connections::Out<ac_int<16> > SC_NAMED(out1);

  SC_CTOR(dut) {
    SC_THREAD(thread1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
    SC_THREAD(thread2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void thread1() {
    bool ping_pong = false;
    in1.Reset();
    sync1.reset_sync_out();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      for (int i=0; i < 8; i++) {
        mem[i + (8 * ping_pong)] = in1.Pop();
      }
      sync1.sync_out(mem);
      ping_pong = !ping_pong;
    }
  }

  void thread2() {
    bool ping_pong = false;
    out1.Reset();
    sync1.reset_sync_in();
    wait();                                 // WAIT

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sync1.sync_in(mem);
      for (int i=0; i < 8; i++) {
        out1.Push(mem[i + (8 * ping_pong)]);
      }
      ping_pong = !ping_pong;
    }
  }

  private:
  Connections::SyncChannel SC_NAMED(sync1); // memory synchronization between threads
#ifdef USE_EXT_ARRAY
  extended_array<ac_int<16>,128> mem{"mem_prehls"};
#else
  ac_shared_array_1D<ac_int<16>, 128> mem;
  // ac_shared<ac_int<16> [128]> mem;
#endif
};


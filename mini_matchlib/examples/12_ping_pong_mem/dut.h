
#pragma once

#include "../include/new_connections.h"

SC_MODULE(dut)
{
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In<sc_uint<16> >  SC_NAMED(in1);
  Connections::Out<sc_uint<16> > SC_NAMED(out1);

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
    sync1.ResetWrite();
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
    sync1.ResetRead();
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
  sc_uint<16> mem[128];
};

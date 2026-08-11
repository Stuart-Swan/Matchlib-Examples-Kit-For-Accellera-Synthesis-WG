
#pragma once

#ifdef __SYNTHESIS__
template <typename T>
class ac_shared
{
public:
  T      d;

  ac_shared() {}
  ac_shared(const ac_shared &s) : d(s.d) {}
  ac_shared(const T &t) : d(t) {}
  template <typename T2> ac_shared(const T2 &t) : d(t) {}

  operator const T &() const   { return d; }
  operator       T &()         { return d; }

  const ac_shared &operator =(const ac_shared &s) { d = s.d; return *this; }
  const T &operator =(const T &t) { d = t; return d; }
  template <typename T2>
  const T &operator =(const T2 &t) { d = t; return d; }
};
#endif

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut)
{
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  msg_in<sc_uint<16> >  SC_NAMED(in1);
  msg_out<sc_uint<16> > SC_NAMED(out1);

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
    in1.reset_pop();
    sync1.reset_push();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      for (int i=0; i < 8; i++) {
        mem[i + (8 * ping_pong)] = in1.pop();
      }
      sync1.sync_write(mem);
      ping_pong = !ping_pong;
    }
  }

  void thread2() {
    bool ping_pong = false;
    out1.reset_push();
    sync1.reset_pop();
    wait();                                 // WAIT

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sync1.sync_read(mem);
      for (int i=0; i < 8; i++) {
        out1.push(mem[i + (8 * ping_pong)]);
      }
      ping_pong = !ping_pong;
    }
  }

  private:
  sync_channel<> SC_NAMED(sync1); // memory synchronization between threads
#ifdef __SYNTHESIS__
  ac_shared<sc_uint<16> [128]> mem;
#else
  sc_uint<16> mem[128];
#endif
};

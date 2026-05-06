// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>
#include "sc_named.h"

#ifdef __SYNTHESIS__
#define CONN_SYNTH_NAME(prefix, nm) ""
#else
#define CONN_SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif

template <class T, int N>
struct fifo_chan {
  Connections::Fifo<T, N> fifo1{CONN_SYNTH_NAME(nm, "_fifo1")};
  Connections::Combinational<T> in1{CONN_SYNTH_NAME(nm, "_in1")};
  Connections::Combinational<T> out1{CONN_SYNTH_NAME(nm, "_out1")};

  std::string nm;

  fifo_chan(const char* s="") : nm(s) {
    fifo1.enq(in1);
    fifo1.deq(out1);
  }

  void ResetWrite() { in1.ResetWrite(); }
  void Push(const T& v) { in1.Push(v); }
  bool PushNB(const T& v) { return in1.PushNB(v); }

  void ResetRead() { out1.ResetRead(); }
  T Pop() { return out1.Pop(); }
  bool PopNB(T& v) { return out1.PopNB(v); }
};


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  typedef ac_int<32, false> T;
  static const int N = 8;

  Connections::In <T> SC_NAMED(in1);
  Connections::Out<T> SC_NAMED(out1);

  fifo_chan<T, N> SC_NAMED(fifo_chan1);

  SC_CTOR(dut) {
    fifo_chan1.fifo1.clk(clk);
    fifo_chan1.fifo1.rst(rst_bar);

    SC_THREAD(main1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(main2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main1() {
    fifo_chan1.ResetWrite();
    in1.Reset();
    wait();  
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { fifo_chan1.Push(in1.Pop()); }
  }

  void main2() {
    fifo_chan1.ResetRead();
    out1.Reset();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { out1.Push(fifo_chan1.Pop()); }
  }
};

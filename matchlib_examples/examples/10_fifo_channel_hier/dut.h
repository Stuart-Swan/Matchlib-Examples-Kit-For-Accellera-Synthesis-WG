// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

template <class T>
class copyio : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<T> SC_NAMED(out1);
  Connections::In <T> SC_NAMED(in1);

  SC_CTOR(copyio) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main() {
    out1.Reset();
    in1.Reset();

    wait();                                 // WAIT

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) { out1.Push(in1.Pop()); }
  }
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

  copyio<T> SC_NAMED(copy1);
  copyio<T> SC_NAMED(copy2);

  Connections::Fifo<T, N> SC_NAMED(fifo1);
  Connections::Combinational<T> SC_NAMED(fifo1_in1);
  Connections::Combinational<T> SC_NAMED(fifo1_out1);


  SC_CTOR(dut) {
    copy1.clk(clk);
    copy1.rst_bar(rst_bar);
    copy1.in1(in1);
    copy1.out1(fifo1_in1);

    fifo1.clk(clk);
    fifo1.rst(rst_bar);
    fifo1.enq(fifo1_in1);
    fifo1.deq(fifo1_out1);

    copy2.clk(clk);
    copy2.rst_bar(rst_bar);
    copy2.in1(fifo1_out1);
    copy2.out1(out1);
  }
};


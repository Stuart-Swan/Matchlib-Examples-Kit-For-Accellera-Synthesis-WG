// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "mc_toolkit_utils.h"

#ifndef CONNECTIONS_SIM_ONLY
#include "ac_blackbox.h"
#endif

template <class T>
class block1 : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<T> SC_NAMED(out1);
  Connections::In <T> SC_NAMED(in1);

  SC_CTOR(block1) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main() {
    out1.Reset();
    in1.Reset();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      T t = in1.Pop();
      out1.Push(t + 0x100);
    }
  }
};

template <class T>
class block2 : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<T> SC_NAMED(out1);
  Connections::In <T> SC_NAMED(in1);

  SC_CTOR(block2) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  void main() {
    out1.Reset();
    in1.Reset();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      T t = in1.Pop();
      out1.Push(t + 0x100);
    }
  }
};

template <class T>
class cdc_fifo : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk1);
  sc_in<bool> SC_NAMED(clk2);
  sc_in<bool> SC_NAMED(rst_bar1);
  sc_in<bool> SC_NAMED(rst_bar2);

  Connections::Out<T> SC_NAMED(out1);
  Connections::In <T> SC_NAMED(in1);

#ifdef CONNECTIONS_SIM_ONLY
  tlm::tlm_fifo<T>    SC_NAMED(fifo1);
#endif

  SC_CTOR(cdc_fifo) {
#ifdef CONNECTIONS_SIM_ONLY
    out1.disable_spawn();
    in1.disable_spawn();

    SC_THREAD(thread1);
    sensitive << clk1.pos();
    async_reset_signal_is(rst_bar1, false);

    SC_THREAD(thread2);
    sensitive << clk2.pos();
    async_reset_signal_is(rst_bar2, false);
#else
       ac_blackbox()
           .entity("cdc_fifo")
           .architecture("cdc_fifo")
           .library("work")
           .verilog_files("cdc_fifo.v")
           .parameter("width", Wrapped<T>::width)
           .end();

#endif
  }

#ifdef CONNECTIONS_SIM_ONLY
  void thread1() {
    // in1.Reset();
    wait();
    while (1) {
      in1.rdy = true;
      do {
        wait();
      } while (!in1.vld);
      in1.rdy = false;
      T tmp;
      bits_to_type_if_needed(tmp, in1.dat);
      fifo1.put(tmp);
    }
  }

  void thread2() {
    // out1.Reset();
    wait();
    while (1) {
      T t = fifo1.get();
      out1.vld = true;
      out1.dat = t;
      do {
        wait();
      } while (!out1.rdy);
      out1.vld = false;
    }
  }
#endif
};

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk1);
  sc_in<bool> SC_NAMED(clk2);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<uint32_t> SC_NAMED(out1);
  Connections::In <uint32_t> SC_NAMED(in1);
  Connections::Combinational<uint32_t> SC_NAMED(chan1);
  Connections::Combinational<uint32_t> SC_NAMED(chan2);

  block1<uint32_t>    SC_NAMED(b1);
  block2<uint32_t>    SC_NAMED(b2);
  cdc_fifo<uint32_t>  SC_NAMED(cdc_fifo1);

  SC_CTOR(dut) {
    b1.clk(clk1);
    b1.rst_bar(rst_bar);
    b1.in1(in1);
    b1.out1(chan1);

    cdc_fifo1.clk1(clk1);
    cdc_fifo1.clk2(clk2);
    cdc_fifo1.rst_bar1(rst_bar);
    cdc_fifo1.rst_bar2(rst_bar);
    cdc_fifo1.in1(chan1);
    cdc_fifo1.out1(chan2);

    b2.clk(clk2);
    b2.rst_bar(rst_bar);
    b2.in1(chan2);
    b2.out1(out1);
  }
};

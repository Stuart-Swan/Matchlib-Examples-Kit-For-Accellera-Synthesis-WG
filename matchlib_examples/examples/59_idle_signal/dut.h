// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>
#include <ac_math.h>

#include "connections_idle.h"
#include "sc_named.h"

#include "clock_gate_module.h"

#define AUTOMATIC_IDLE 1


class sub : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<uint32> SC_NAMED(out1);
  Connections::In <uint32> SC_NAMED(in1);
  Connections::In <uint32> SC_NAMED(in2);
#ifdef AUTOMATIC_IDLE
#pragma hls_idle
#endif
  sc_out<bool>  SC_NAMED(idle);

#ifndef AUTOMATIC_IDLE
  static const int MAX_PIPE_STAGES = 15;
  typedef ac_int<ac::nbits<MAX_PIPE_STAGES>::val, false> CNT_T;

  SC_SIG(CNT_T, in1_cnt);
  SC_SIG(CNT_T, in2_cnt);
  SC_SIG(CNT_T, out1_cnt);
#endif

  SC_CTOR(sub) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifndef AUTOMATIC_IDLE
    SC_THREAD(out1_thread);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(in1_thread);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(in2_thread);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_METHOD(idle_thread);
    sensitive << in1_cnt << in2_cnt << out1_cnt << in1.rdy << in1.vld << in2.rdy << in2.vld;
#endif
  }

private:

#ifndef AUTOMATIC_IDLE
  void idle_thread() {
    idle = EQUAL_COUNTS(in1, out1) &&
           EQUAL_COUNTS(in2, out1) &&
           ( IN_STALLED(in1) || IN_STALLED(in2)) ;
  }

  COUNTER_THREAD(out1);
  COUNTER_THREAD(in1);
  COUNTER_THREAD(in2);
#endif

  void main() {
    out1.Reset();
    in1.Reset();
    in2.Reset();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      ac_int<16, false> t1 = in1.Pop();
      ac_int<16, false> t2 = in2.Pop();
      ac_int<32, false> sqrt = 0;
      ac_math::ac_sqrt(t1*t2, sqrt);  // internal loop is unrolled in catapult .tcl file
      out1.Push(sqrt);
    }
  }
};

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::Out<uint32> SC_NAMED(out1);
  Connections::In <uint32> SC_NAMED(in1);
  Connections::In <uint32> SC_NAMED(in2);
  SC_SIG(bool, idle);
  SC_SIG(bool, gated_clk);
  clock_gate_module SC_NAMED(clock_gate_module1);

  sub SC_NAMED(sub1);

  SC_CTOR(dut) {
    clock_gate_module1.idle_in(idle);
    clock_gate_module1.clk_in(clk);
    clock_gate_module1.clk_out(gated_clk);

    sub1.clk(gated_clk);
    sub1.rst_bar(rst_bar);
    sub1.out1(out1);
    sub1.in1(in1);
    sub1.in2(in2);
    sub1.idle(idle);
  }
};

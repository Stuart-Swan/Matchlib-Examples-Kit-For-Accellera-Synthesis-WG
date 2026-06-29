// INSERT_EULA_COPYRIGHT: 2020

#ifndef _INCLUDED_RECONVERGENCE_H_
#define _INCLUDED_RECONVERGENCE_H_

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

class divergence : public sc_module
{
public:
  sc_in<bool>                        SC_NAMED(clk);
  sc_in<bool>                        SC_NAMED(rstn);

  msg_in <sc_uint<16>> SC_NAMED(din);
  msg_out<sc_uint<16>> SC_NAMED(dout0);
  msg_out<sc_uint<16>> SC_NAMED(dout1);

  SC_CTOR(divergence) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

private:
  void run() {
    din.reset_pop();
    dout0.reset_push();
    dout1.reset_push();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<16> tmp = din.pop();
      dout0.push(tmp);
      dout1.push(tmp);
    }
  }
};

class block0 : public sc_module
{
public:
  sc_in<bool>                        SC_NAMED(clk);
  sc_in<bool>                        SC_NAMED(rstn);

  msg_in <sc_uint<16>> SC_NAMED(din);
  msg_out<sc_uint<16>> SC_NAMED(dout);

  SC_CTOR(block0) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

private:
  void run() {
    din.reset_pop();
    dout.reset_push();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<16> tmp = din.pop();
      dout.push(tmp);
    }
  }
};

class block1 : public sc_module
{
public:
  sc_in<bool>                        SC_NAMED(clk);
  sc_in<bool>                        SC_NAMED(rstn);

  msg_in <sc_uint<16>> SC_NAMED(din);
  msg_out<sc_uint<16>> SC_NAMED(dout);

  SC_CTOR(block1) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

private:
  void run() {
    din.reset_pop();
    dout.reset_push();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<16> tmp = din.pop();
      dout.push(tmp);
    }
  }
};

class reconvergence : public sc_module
{
public:
  sc_in<bool>                        SC_NAMED(clk);
  sc_in<bool>                        SC_NAMED(rstn);

  msg_in <sc_uint<16>> SC_NAMED(din0);
  msg_in<sc_uint<16>>  SC_NAMED(din1);
  msg_out<sc_uint<16>> SC_NAMED(dout);

  SC_CTOR(reconvergence) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

private:
  void run() {
    din0.reset_pop();
    din1.reset_pop();
    dout.reset_push();
    wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      sc_uint<16> tmp0 = din0.pop();
      sc_uint<16> tmp1 = din1.pop();
      dout.push(tmp0 + tmp1);
    }
  }
};

#pragma hls_design top
class top : public sc_module
{
public:
  sc_in<bool>                        SC_NAMED(clk);
  sc_in<bool>                        SC_NAMED(rstn);
  msg_in <sc_uint<16>> SC_NAMED(din);
  msg_out<sc_uint<16>> SC_NAMED(dout);

  SC_CTOR(top) {

    divergence_inst.clk(clk);
    divergence_inst.rstn(rstn);
    divergence_inst.din(din);
    divergence_inst.dout0(dout0);
    divergence_inst.dout1(dout1);

    block0_inst.clk(clk);
    block0_inst.rstn(rstn);
    block0_inst.din(dout0);
    block0_inst.dout(bout0);

    block1_inst.clk(clk);
    block1_inst.rstn(rstn);
    block1_inst.din(dout1);
    block1_inst.dout(bout1);

    reconvergence_inst.clk(clk);
    reconvergence_inst.rstn(rstn);
    reconvergence_inst.din0(bout0);
    reconvergence_inst.din1(bout1);
    reconvergence_inst.dout(dout);
  }

private: // Instances and interconnect
  divergence                                   SC_NAMED(divergence_inst);
  block0                                       SC_NAMED(block0_inst);
  block1                                       SC_NAMED(block1_inst);
  reconvergence                                SC_NAMED(reconvergence_inst);
  msg_channel<sc_uint<16>> SC_NAMED(dout0);
  msg_channel<sc_uint<16>> SC_NAMED(dout1);
  msg_channel<sc_uint<16>> SC_NAMED(bout0);
  msg_channel<sc_uint<16>> SC_NAMED(bout1);
};
#endif


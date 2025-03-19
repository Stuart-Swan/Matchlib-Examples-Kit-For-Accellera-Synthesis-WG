// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "mem_with_stall.h"

typedef uint8_t addr_t;
typedef uint8_t data_t;

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In<mem_with_stall_req<addr_t, data_t>> SC_NAMED(in1);
  Connections::Out<mem_with_stall_rsp<data_t>> SC_NAMED(out1);
  mem_with_stall_out<addr_t, data_t> SC_NAMED(mem1);  // out port using mem_with_stall protocol

  mem_with_stall_out_xact<addr_t, data_t> SC_NAMED(mem1_xact); // convert Push/Pop to mem_with_stall

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    mem1_xact.bind(clk, rst_bar, mem1);
  }

private:

  void main() {
    mem1_xact.req_chan.ResetWrite();
    mem1_xact.rsp_chan.ResetRead();
    in1.Reset();
    out1.Reset();
    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      auto i1 = in1.Pop();
      if (i1.write_en) {
        mem1_xact[i1.address] = i1.write_data;
      } else if (i1.read_en) {
        mem_with_stall_rsp<data_t> rsp;
        // Note: need to use "cycle set" in go_hls.tcl to separate Pop and Push here..
        rsp.read_data = mem1_xact[i1.address];
        out1.Push(rsp);
      }
    }
  }
};

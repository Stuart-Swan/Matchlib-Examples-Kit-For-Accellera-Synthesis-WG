// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <ac_math.h>
#include <mc_connections.h>
#include <ac_assert_no_x_local.h>
#include "axi4_segment.h"


typedef axi::axi4_segment<axi::cfg::standard> local_axi;


#pragma hls_design top
class dut : public sc_module, public local_axi
{
public:
  sc_in<bool>                             SC_NAMED(clk);
  sc_in<bool>                             SC_NAMED(rst_bar);
  Connections::Out<uint32>                SC_NAMED(out1);
  Connections::In<uint32>                 SC_NAMED(in1);
  Connections::In<uint32>                 SC_NAMED(in2);
  Connections::In<uint32>                 SC_NAMED(in3);

  r_master<> SC_NAMED(r_master0);
  w_master<> SC_NAMED(w_master0);

  uint32 mem[0x1000];

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    w_segment0.bind(clk, rst_bar, w_master0);
    r_segment0.bind(clk, rst_bar, r_master0);

  }

private:

  axi4_w_segment_cfg<local_axi> SC_NAMED(w_segment0);
  axi4_r_segment_cfg<local_axi> SC_NAMED(r_segment0);

  void main() {
    in1.Reset();
    in2.Reset();
    in3.Reset();
    out1.Reset();
    w_segment0.Reset();
    r_segment0.Reset();
    r_master0.r.Reset();

    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      uint32 t1 = in1.Pop();
      uint32 t2 = in2.Pop();
      uint32 t3 = in3.Pop();

      if (t1 == 0) {
#ifndef USE_AXI_MEM
        mem[t3] = t2;
#else
        ex_aw_payload aw;
        aw.ex_len = 0;
        aw.addr = t3;
        w_segment0.ex_aw_chan.Push(aw);
        w_payload w;
        w.data = t2;
        w_segment0.w_chan.Push(w);
        w_segment0.b_chan.Pop();
#endif
      } else {
        ac_int<32, false> prod = (t1 * t2);
        ac_int<32, false> sqrt = 0;
        ac_math::ac_sqrt(prod, sqrt);

        uint32 rdata;
#ifndef USE_AXI_MEM
        rdata = mem[t3];
#else
        ex_ar_payload ar;
        ar.ex_len = 0;
        ar.addr = t3;
        r_segment0.ex_ar_chan.Push(ar);
        r_payload r = r_master0.r.Pop();
        rdata = r.data;
#endif
        assert_no_x(rdata);
        uint32 result = sqrt + rdata;
        out1.Push(result);
      }
    }
  }
};

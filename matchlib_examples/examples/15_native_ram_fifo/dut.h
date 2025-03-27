// INSERT_EULA_COPYRIGHT: 2020

#pragma once
#include "mc_connections.h"
#include <nvhls_connections_buffered_ports.h>


#include "ac_shared_array_1D.h"

extern sc_trace_file* trace_file_ptr;

// FIFO that uses a RAM for storage. The RAM is modeled as a normal C array so
// that it can be treated as such during HLS.

template <class T, int N>
class mm_fifo: public sc_module {
public:
    sc_in<bool> CCS_INIT_S1(clk);
    sc_in<bool> CCS_INIT_S1(rst_n);
    Connections::In <T> CCS_INIT_S1(i_dat);
    Connections::Out<T> CCS_INIT_S1(o_dat);
    Connections::SyncChannel CCS_INIT_S1(wr_sync);
    Connections::SyncChannel CCS_INIT_S1(rd_sync);

    ac_shared_array_1D<T, N> mem;

    SC_VAR(T, rd_dat);
    SC_VAR(T, wr_dat);
    static const int AddrWidth = nvhls::index_width<N>::val;
    typedef ac_int<AddrWidth, false> addr_t;
    SC_SIG(addr_t, wr_ptr);
    SC_SIG(addr_t, rd_ptr);
    SC_SIG(bool, wr_ptr_vld);

    SC_CTOR(mm_fifo) {
        SC_THREAD(rd_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst_n, false);

        SC_THREAD(wr_process);
        sensitive <<  clk.pos();
        async_reset_signal_is(rst_n, false);

        SC_METHOD(rdwr_method);
        sensitive << wr_ptr << rd_ptr << wr_ptr_vld;

#ifndef __SYNTHESIS__
        sc_trace(trace_file_ptr, wr_ptr, "wr_ptr");
        sc_trace(trace_file_ptr, rd_ptr, "rd_ptr");
#endif
    }

    void rdwr_method() {
/*
      if (wr_ptr_vld && (addr_t(wr_ptr.read() + 1) == rd_ptr.read()))
        CCS_LOG("FULL");

      if (!wr_ptr_vld.read() || (wr_ptr.read()  == addr_t(rd_ptr.read() + 0) ))
        CCS_LOG("EMPTY");
*/

      wr_sync.rdy = !(addr_t(wr_ptr.read() + 1) == rd_ptr.read());
      rd_sync.vld = !(!wr_ptr_vld.read() || (wr_ptr.read()  == addr_t(rd_ptr.read() + 0) ));
    }

    void wr_process() {
      wr_ptr = 0;
      wr_ptr_vld = 0;
      i_dat.Reset();
      wr_sync.reset_sync_out();
      wait();
      ac_int<AddrWidth+1, false> adr=0;
      #pragma hls_pipeline_init_interval 1
      #pragma pipeline_stall_mode flush
      while (1) {     
        wr_dat = i_dat.Pop();
        mem[adr] = wr_dat;
        wr_ptr = adr;
        wr_ptr_vld = 1;
        ++adr;
        if (adr == N)
          adr = 0;
        wr_sync.sync_out(mem);
      }
    }

    void rd_process() {
        rd_ptr = 0;
        o_dat.Reset();        
        rd_sync.reset_sync_in();
        wait();
        ac_int<AddrWidth+1, false> adr=0;
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush    
        while (1) {
            rd_ptr = adr;
            rd_sync.sync_in(mem);
            rd_dat = mem[adr];
            ++adr;
            if (adr == N)
              adr = 0;
            o_dat.Push(rd_dat);
        }
    }
};


#pragma hls_design top
class dut: public sc_module {
 public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);
  typedef ac_int<32, false> T;
  static const int N = 32;
  Connections::In <T> CCS_INIT_S1(in1);
  Connections::Out<T> CCS_INIT_S1(out1);
  mm_fifo<T, N>   CCS_INIT_S1(fifo1);
  SC_CTOR(dut)
  {
    fifo1.clk(clk);
    fifo1.rst_n(rst_bar);
    fifo1.i_dat(in1);
    fifo1.o_dat(out1);
  }
  
};

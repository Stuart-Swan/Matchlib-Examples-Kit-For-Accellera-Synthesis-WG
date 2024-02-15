// INSERT_EULA_COPYRIGHT: 2020

#pragma once
#include "mc_connections.h"

template <class T, int N>
class mm_fifo: public sc_module {
public:
    sc_in<bool> CCS_INIT_S1(clk);
    sc_in<bool> CCS_INIT_S1(rst_n);
    Connections::In <T> CCS_INIT_S1(i_dat);
    Connections::Out<T> CCS_INIT_S1(o_dat);

    sc_fifo<T> fifo0;
    
    SC_VAR(T, rd_dat);
    SC_VAR(T, wr_dat);
    SC_SIG(sc_uint<16>,rd_idx);
    SC_SIG(sc_uint<16>,wr_idx);
    SC_HAS_PROCESS(mm_fifo);
    mm_fifo(sc_module_name name)
        : sc_module(name), fifo0(N){
        SC_CTHREAD(rd_process,  clk.pos()); async_reset_signal_is(rst_n, false);
        SC_CTHREAD(wr_process,  clk.pos()); async_reset_signal_is(rst_n, false);
    }
   
 
    void wr_process() {
      i_dat.Reset();
      wait();
      #pragma hls_pipeline_init_interval 1
      #pragma pipeline_stall_mode flush
      while (1) {     
        wr_dat = i_dat.Pop();
        fifo0.write(wr_dat);
      }
    }

    void rd_process() {
        o_dat.Reset();        
        wait();
    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush    
        while (1) {
            rd_dat = fifo0.read();
            o_dat.Push(rd_dat);
        }
    }
};


#pragma hls_design top
class dut: public sc_module {
 public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);
  typedef sc_uint<32> T;
  static const int N = 4096;
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

// Auto generated on: Thu Sep  5 15:38:00 2024

// This file wraps the post-HLS RTL model to enable instantiation in an SC testbench
//  with the same SC interface as the original SC DUT

#include <TypeToBits.h>
#include "dma.h"

class dma_rtl_proxy_type : public sc_foreign_module {
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);
  sc_in<bool> CCS_INIT_S1(r_master0_ar_rdy);
  sc_out<bool> CCS_INIT_S1(r_master0_ar_vld);
  sc_out<sc_lv<44>> CCS_INIT_S1(r_master0_ar_dat);
  sc_out<bool> CCS_INIT_S1(r_master0_r_rdy);
  sc_in<bool> CCS_INIT_S1(r_master0_r_vld);
  sc_in<sc_lv<71>> CCS_INIT_S1(r_master0_r_dat);
  sc_in<bool> CCS_INIT_S1(w_master0_aw_rdy);
  sc_out<bool> CCS_INIT_S1(w_master0_aw_vld);
  sc_out<sc_lv<44>> CCS_INIT_S1(w_master0_aw_dat);
  sc_in<bool> CCS_INIT_S1(w_master0_w_rdy);
  sc_out<bool> CCS_INIT_S1(w_master0_w_vld);
  sc_out<sc_lv<73>> CCS_INIT_S1(w_master0_w_dat);
  sc_out<bool> CCS_INIT_S1(w_master0_b_rdy);
  sc_in<bool> CCS_INIT_S1(w_master0_b_vld);
  sc_in<sc_lv<6>> CCS_INIT_S1(w_master0_b_dat);
  sc_out<bool> CCS_INIT_S1(r_slave0_ar_rdy);
  sc_in<bool> CCS_INIT_S1(r_slave0_ar_vld);
  sc_in<sc_lv<44>> CCS_INIT_S1(r_slave0_ar_dat);
  sc_in<bool> CCS_INIT_S1(r_slave0_r_rdy);
  sc_out<bool> CCS_INIT_S1(r_slave0_r_vld);
  sc_out<sc_lv<71>> CCS_INIT_S1(r_slave0_r_dat);
  sc_out<bool> CCS_INIT_S1(w_slave0_aw_rdy);
  sc_in<bool> CCS_INIT_S1(w_slave0_aw_vld);
  sc_in<sc_lv<44>> CCS_INIT_S1(w_slave0_aw_dat);
  sc_out<bool> CCS_INIT_S1(w_slave0_w_rdy);
  sc_in<bool> CCS_INIT_S1(w_slave0_w_vld);
  sc_in<sc_lv<73>> CCS_INIT_S1(w_slave0_w_dat);
  sc_in<bool> CCS_INIT_S1(w_slave0_b_rdy);
  sc_out<bool> CCS_INIT_S1(w_slave0_b_vld);
  sc_out<sc_lv<6>> CCS_INIT_S1(w_slave0_b_dat);
  sc_in<bool> CCS_INIT_S1(dma_done_rdy);
  sc_out<bool> CCS_INIT_S1(dma_done_vld);
  sc_out<sc_lv<1>> CCS_INIT_S1(dma_done_dat);
  sc_in<bool> CCS_INIT_S1(dma_dbg_rdy);
  sc_out<bool> CCS_INIT_S1(dma_dbg_vld);
  sc_out<sc_lv<32>> CCS_INIT_S1(dma_dbg_dat);

  dma_rtl_proxy_type(sc_module_name nm , const char* hdl_name="dma_wrap_rtl") 
    : sc_foreign_module(nm) {
     elaborate_foreign_module(hdl_name, 0, (const char**)0); 
  }
};


class dma_wrap_rtl : public sc_module {
public:
  dma& dma_inst;

  decltype(dma_inst.clk) CCS_INIT_S1(clk);
  decltype(dma_inst.rst_bar) CCS_INIT_S1(rst_bar);
  decltype(dma_inst.r_master0) CCS_INIT_S1(r_master0);
  decltype(dma_inst.w_master0) CCS_INIT_S1(w_master0);
  decltype(dma_inst.r_slave0) CCS_INIT_S1(r_slave0);
  decltype(dma_inst.w_slave0) CCS_INIT_S1(w_slave0);
  decltype(dma_inst.dma_done) CCS_INIT_S1(dma_done);
  decltype(dma_inst.dma_dbg) CCS_INIT_S1(dma_dbg);
  

  template <class T> struct type_info { };

  template <class T> struct type_info<sc_in<T>> {
    typedef T data_type;
    static const int width = Wrapped<data_type>::width;
    typedef sc_lv<width> sc_lv_type;
    static const bool is_sc_out = 0;
  };

  template <class T> struct type_info<sc_out<T>> {
    typedef T data_type;
    static const int width = Wrapped<data_type>::width;
    typedef sc_lv<width> sc_lv_type;
    static const bool is_sc_out = 1;
  };

  typedef type_info<decltype(dma_inst.clk)> type_info_clk;
  typedef type_info<decltype(dma_inst.rst_bar)> type_info_rst_bar;
  typedef type_info<decltype(dma_inst.r_master0.ar.rdy)> type_info_r_master0_ar_rdy;
  typedef type_info<decltype(dma_inst.r_master0.ar.vld)> type_info_r_master0_ar_vld;
  typedef type_info<decltype(dma_inst.r_master0.ar.dat)> type_info_r_master0_ar_dat;
  typedef type_info<decltype(dma_inst.r_master0.r.rdy)> type_info_r_master0_r_rdy;
  typedef type_info<decltype(dma_inst.r_master0.r.vld)> type_info_r_master0_r_vld;
  typedef type_info<decltype(dma_inst.r_master0.r.dat)> type_info_r_master0_r_dat;
  typedef type_info<decltype(dma_inst.w_master0.aw.rdy)> type_info_w_master0_aw_rdy;
  typedef type_info<decltype(dma_inst.w_master0.aw.vld)> type_info_w_master0_aw_vld;
  typedef type_info<decltype(dma_inst.w_master0.aw.dat)> type_info_w_master0_aw_dat;
  typedef type_info<decltype(dma_inst.w_master0.w.rdy)> type_info_w_master0_w_rdy;
  typedef type_info<decltype(dma_inst.w_master0.w.vld)> type_info_w_master0_w_vld;
  typedef type_info<decltype(dma_inst.w_master0.w.dat)> type_info_w_master0_w_dat;
  typedef type_info<decltype(dma_inst.w_master0.b.rdy)> type_info_w_master0_b_rdy;
  typedef type_info<decltype(dma_inst.w_master0.b.vld)> type_info_w_master0_b_vld;
  typedef type_info<decltype(dma_inst.w_master0.b.dat)> type_info_w_master0_b_dat;
  typedef type_info<decltype(dma_inst.r_slave0.ar.rdy)> type_info_r_slave0_ar_rdy;
  typedef type_info<decltype(dma_inst.r_slave0.ar.vld)> type_info_r_slave0_ar_vld;
  typedef type_info<decltype(dma_inst.r_slave0.ar.dat)> type_info_r_slave0_ar_dat;
  typedef type_info<decltype(dma_inst.r_slave0.r.rdy)> type_info_r_slave0_r_rdy;
  typedef type_info<decltype(dma_inst.r_slave0.r.vld)> type_info_r_slave0_r_vld;
  typedef type_info<decltype(dma_inst.r_slave0.r.dat)> type_info_r_slave0_r_dat;
  typedef type_info<decltype(dma_inst.w_slave0.aw.rdy)> type_info_w_slave0_aw_rdy;
  typedef type_info<decltype(dma_inst.w_slave0.aw.vld)> type_info_w_slave0_aw_vld;
  typedef type_info<decltype(dma_inst.w_slave0.aw.dat)> type_info_w_slave0_aw_dat;
  typedef type_info<decltype(dma_inst.w_slave0.w.rdy)> type_info_w_slave0_w_rdy;
  typedef type_info<decltype(dma_inst.w_slave0.w.vld)> type_info_w_slave0_w_vld;
  typedef type_info<decltype(dma_inst.w_slave0.w.dat)> type_info_w_slave0_w_dat;
  typedef type_info<decltype(dma_inst.w_slave0.b.rdy)> type_info_w_slave0_b_rdy;
  typedef type_info<decltype(dma_inst.w_slave0.b.vld)> type_info_w_slave0_b_vld;
  typedef type_info<decltype(dma_inst.w_slave0.b.dat)> type_info_w_slave0_b_dat;
  typedef type_info<decltype(dma_inst.dma_done.rdy)> type_info_dma_done_rdy;
  typedef type_info<decltype(dma_inst.dma_done.vld)> type_info_dma_done_vld;
  typedef type_info<decltype(dma_inst.dma_done.dat)> type_info_dma_done_dat;
  typedef type_info<decltype(dma_inst.dma_dbg.rdy)> type_info_dma_dbg_rdy;
  typedef type_info<decltype(dma_inst.dma_dbg.vld)> type_info_dma_dbg_vld;
  typedef type_info<decltype(dma_inst.dma_dbg.dat)> type_info_dma_dbg_dat;

  sc_signal<type_info_r_master0_ar_dat::sc_lv_type> CCS_INIT_S1(sig_r_master0_ar_dat);
  sc_signal<type_info_r_master0_r_dat::sc_lv_type> CCS_INIT_S1(sig_r_master0_r_dat);
  sc_signal<type_info_w_master0_aw_dat::sc_lv_type> CCS_INIT_S1(sig_w_master0_aw_dat);
  sc_signal<type_info_w_master0_w_dat::sc_lv_type> CCS_INIT_S1(sig_w_master0_w_dat);
  sc_signal<type_info_w_master0_b_dat::sc_lv_type> CCS_INIT_S1(sig_w_master0_b_dat);
  sc_signal<type_info_r_slave0_ar_dat::sc_lv_type> CCS_INIT_S1(sig_r_slave0_ar_dat);
  sc_signal<type_info_r_slave0_r_dat::sc_lv_type> CCS_INIT_S1(sig_r_slave0_r_dat);
  sc_signal<type_info_w_slave0_aw_dat::sc_lv_type> CCS_INIT_S1(sig_w_slave0_aw_dat);
  sc_signal<type_info_w_slave0_w_dat::sc_lv_type> CCS_INIT_S1(sig_w_slave0_w_dat);
  sc_signal<type_info_w_slave0_b_dat::sc_lv_type> CCS_INIT_S1(sig_w_slave0_b_dat);
  sc_signal<type_info_dma_done_dat::sc_lv_type> CCS_INIT_S1(sig_dma_done_dat);
  sc_signal<type_info_dma_dbg_dat::sc_lv_type> CCS_INIT_S1(sig_dma_dbg_dat);

  dma_rtl_proxy_type CCS_INIT_S1(dma_rtl_inst);
  SC_HAS_PROCESS(dma_wrap_rtl);

  dma_wrap_rtl(sc_module_name nm) : dma_inst(*(dma*)0){

    dma_done.disable_spawn();
    dma_dbg.disable_spawn();

    SC_METHOD(method_r_master0_ar_dat); sensitive << sig_r_master0_ar_dat; dont_initialize();
    SC_METHOD(method_r_master0_r_dat); sensitive << r_master0.r.dat;
    SC_METHOD(method_w_master0_aw_dat); sensitive << sig_w_master0_aw_dat; dont_initialize();
    SC_METHOD(method_w_master0_w_dat); sensitive << sig_w_master0_w_dat; dont_initialize();
    SC_METHOD(method_w_master0_b_dat); sensitive << w_master0.b.dat;
    SC_METHOD(method_r_slave0_ar_dat); sensitive << r_slave0.ar.dat;
    SC_METHOD(method_r_slave0_r_dat); sensitive << sig_r_slave0_r_dat; dont_initialize();
    SC_METHOD(method_w_slave0_aw_dat); sensitive << w_slave0.aw.dat;
    SC_METHOD(method_w_slave0_w_dat); sensitive << w_slave0.w.dat;
    SC_METHOD(method_w_slave0_b_dat); sensitive << sig_w_slave0_b_dat; dont_initialize();
    SC_METHOD(method_dma_done_dat); sensitive << sig_dma_done_dat; dont_initialize();
    SC_METHOD(method_dma_dbg_dat); sensitive << sig_dma_dbg_dat; dont_initialize();

    dma_rtl_inst.clk(clk);
    dma_rtl_inst.rst_bar(rst_bar);
    dma_rtl_inst.r_master0_ar_rdy(r_master0.ar.rdy);
    dma_rtl_inst.r_master0_ar_vld(r_master0.ar.vld);
    dma_rtl_inst.r_master0_ar_dat(sig_r_master0_ar_dat);
    dma_rtl_inst.r_master0_r_rdy(r_master0.r.rdy);
    dma_rtl_inst.r_master0_r_vld(r_master0.r.vld);
    dma_rtl_inst.r_master0_r_dat(sig_r_master0_r_dat);
    dma_rtl_inst.w_master0_aw_rdy(w_master0.aw.rdy);
    dma_rtl_inst.w_master0_aw_vld(w_master0.aw.vld);
    dma_rtl_inst.w_master0_aw_dat(sig_w_master0_aw_dat);
    dma_rtl_inst.w_master0_w_rdy(w_master0.w.rdy);
    dma_rtl_inst.w_master0_w_vld(w_master0.w.vld);
    dma_rtl_inst.w_master0_w_dat(sig_w_master0_w_dat);
    dma_rtl_inst.w_master0_b_rdy(w_master0.b.rdy);
    dma_rtl_inst.w_master0_b_vld(w_master0.b.vld);
    dma_rtl_inst.w_master0_b_dat(sig_w_master0_b_dat);
    dma_rtl_inst.r_slave0_ar_rdy(r_slave0.ar.rdy);
    dma_rtl_inst.r_slave0_ar_vld(r_slave0.ar.vld);
    dma_rtl_inst.r_slave0_ar_dat(sig_r_slave0_ar_dat);
    dma_rtl_inst.r_slave0_r_rdy(r_slave0.r.rdy);
    dma_rtl_inst.r_slave0_r_vld(r_slave0.r.vld);
    dma_rtl_inst.r_slave0_r_dat(sig_r_slave0_r_dat);
    dma_rtl_inst.w_slave0_aw_rdy(w_slave0.aw.rdy);
    dma_rtl_inst.w_slave0_aw_vld(w_slave0.aw.vld);
    dma_rtl_inst.w_slave0_aw_dat(sig_w_slave0_aw_dat);
    dma_rtl_inst.w_slave0_w_rdy(w_slave0.w.rdy);
    dma_rtl_inst.w_slave0_w_vld(w_slave0.w.vld);
    dma_rtl_inst.w_slave0_w_dat(sig_w_slave0_w_dat);
    dma_rtl_inst.w_slave0_b_rdy(w_slave0.b.rdy);
    dma_rtl_inst.w_slave0_b_vld(w_slave0.b.vld);
    dma_rtl_inst.w_slave0_b_dat(sig_w_slave0_b_dat);
    dma_rtl_inst.dma_done_rdy(dma_done.rdy);
    dma_rtl_inst.dma_done_vld(dma_done.vld);
    dma_rtl_inst.dma_done_dat(sig_dma_done_dat);
    dma_rtl_inst.dma_dbg_rdy(dma_dbg.rdy);
    dma_rtl_inst.dma_dbg_vld(dma_dbg.vld);
    dma_rtl_inst.dma_dbg_dat(sig_dma_dbg_dat);
  }

  void method_r_master0_ar_dat(){
    typename type_info_r_master0_ar_dat::data_type obj;
    typename type_info_r_master0_ar_dat::sc_lv_type lv;
    lv = sig_r_master0_ar_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    r_master0.ar.dat = obj;
  }
  void method_r_master0_r_dat(){
    typename type_info_r_master0_r_dat::data_type obj;
    typename type_info_r_master0_r_dat::sc_lv_type lv;
    obj = r_master0.r.dat;
    lv = TypeToBits(obj);
    sig_r_master0_r_dat = lv;
  }
  void method_w_master0_aw_dat(){
    typename type_info_w_master0_aw_dat::data_type obj;
    typename type_info_w_master0_aw_dat::sc_lv_type lv;
    lv = sig_w_master0_aw_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    w_master0.aw.dat = obj;
  }
  void method_w_master0_w_dat(){
    typename type_info_w_master0_w_dat::data_type obj;
    typename type_info_w_master0_w_dat::sc_lv_type lv;
    lv = sig_w_master0_w_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    w_master0.w.dat = obj;
  }
  void method_w_master0_b_dat(){
    typename type_info_w_master0_b_dat::data_type obj;
    typename type_info_w_master0_b_dat::sc_lv_type lv;
    obj = w_master0.b.dat;
    lv = TypeToBits(obj);
    sig_w_master0_b_dat = lv;
  }
  void method_r_slave0_ar_dat(){
    typename type_info_r_slave0_ar_dat::data_type obj;
    typename type_info_r_slave0_ar_dat::sc_lv_type lv;
    obj = r_slave0.ar.dat;
    lv = TypeToBits(obj);
    sig_r_slave0_ar_dat = lv;
  }
  void method_r_slave0_r_dat(){
    typename type_info_r_slave0_r_dat::data_type obj;
    typename type_info_r_slave0_r_dat::sc_lv_type lv;
    lv = sig_r_slave0_r_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    r_slave0.r.dat = obj;
  }
  void method_w_slave0_aw_dat(){
    typename type_info_w_slave0_aw_dat::data_type obj;
    typename type_info_w_slave0_aw_dat::sc_lv_type lv;
    obj = w_slave0.aw.dat;
    lv = TypeToBits(obj);
    sig_w_slave0_aw_dat = lv;
  }
  void method_w_slave0_w_dat(){
    typename type_info_w_slave0_w_dat::data_type obj;
    typename type_info_w_slave0_w_dat::sc_lv_type lv;
    obj = w_slave0.w.dat;
    lv = TypeToBits(obj);
    sig_w_slave0_w_dat = lv;
  }
  void method_w_slave0_b_dat(){
    typename type_info_w_slave0_b_dat::data_type obj;
    typename type_info_w_slave0_b_dat::sc_lv_type lv;
    lv = sig_w_slave0_b_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    w_slave0.b.dat = obj;
  }
  void method_dma_done_dat(){
    typename type_info_dma_done_dat::data_type obj;
    typename type_info_dma_done_dat::sc_lv_type lv;
    lv = sig_dma_done_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    dma_done.dat = obj;
  }
  void method_dma_dbg_dat(){
    typename type_info_dma_dbg_dat::data_type obj;
    typename type_info_dma_dbg_dat::sc_lv_type lv;
    lv = sig_dma_dbg_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    dma_dbg.dat = obj;
  }

};

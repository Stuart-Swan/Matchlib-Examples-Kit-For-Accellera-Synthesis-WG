// Auto generated on: Thu Sep  5 15:38:00 2024

// This file is an SC wrapper of the pre-HLS model to an HDL simulator

#include "dma.h"

extern sc_trace_file* trace_file_ptr;

class dma_wrap : public sc_module {
  #include "mc_toolkit_utils.h"
public:
  dma SC_NAMED(dma_inst);

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

  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);
  sc_in<bool> SC_NAMED(r_master0_ar_rdy);
  sc_out<bool> SC_NAMED(r_master0_ar_vld);
  sc_out<sc_lv<type_info_r_master0_ar_dat::width>> SC_NAMED(r_master0_ar_dat);
  sc_out<bool> SC_NAMED(r_master0_r_rdy);
  sc_in<bool> SC_NAMED(r_master0_r_vld);
  sc_in<sc_lv<type_info_r_master0_r_dat::width>> SC_NAMED(r_master0_r_dat);
  sc_in<bool> SC_NAMED(w_master0_aw_rdy);
  sc_out<bool> SC_NAMED(w_master0_aw_vld);
  sc_out<sc_lv<type_info_w_master0_aw_dat::width>> SC_NAMED(w_master0_aw_dat);
  sc_in<bool> SC_NAMED(w_master0_w_rdy);
  sc_out<bool> SC_NAMED(w_master0_w_vld);
  sc_out<sc_lv<type_info_w_master0_w_dat::width>> SC_NAMED(w_master0_w_dat);
  sc_out<bool> SC_NAMED(w_master0_b_rdy);
  sc_in<bool> SC_NAMED(w_master0_b_vld);
  sc_in<sc_lv<type_info_w_master0_b_dat::width>> SC_NAMED(w_master0_b_dat);
  sc_out<bool> SC_NAMED(r_slave0_ar_rdy);
  sc_in<bool> SC_NAMED(r_slave0_ar_vld);
  sc_in<sc_lv<type_info_r_slave0_ar_dat::width>> SC_NAMED(r_slave0_ar_dat);
  sc_in<bool> SC_NAMED(r_slave0_r_rdy);
  sc_out<bool> SC_NAMED(r_slave0_r_vld);
  sc_out<sc_lv<type_info_r_slave0_r_dat::width>> SC_NAMED(r_slave0_r_dat);
  sc_out<bool> SC_NAMED(w_slave0_aw_rdy);
  sc_in<bool> SC_NAMED(w_slave0_aw_vld);
  sc_in<sc_lv<type_info_w_slave0_aw_dat::width>> SC_NAMED(w_slave0_aw_dat);
  sc_out<bool> SC_NAMED(w_slave0_w_rdy);
  sc_in<bool> SC_NAMED(w_slave0_w_vld);
  sc_in<sc_lv<type_info_w_slave0_w_dat::width>> SC_NAMED(w_slave0_w_dat);
  sc_in<bool> SC_NAMED(w_slave0_b_rdy);
  sc_out<bool> SC_NAMED(w_slave0_b_vld);
  sc_out<sc_lv<type_info_w_slave0_b_dat::width>> SC_NAMED(w_slave0_b_dat);
  sc_in<bool> SC_NAMED(dma_done_rdy);
  sc_out<bool> SC_NAMED(dma_done_vld);
  sc_out<sc_lv<type_info_dma_done_dat::width>> SC_NAMED(dma_done_dat);
  sc_in<bool> SC_NAMED(dma_dbg_rdy);
  sc_out<bool> SC_NAMED(dma_dbg_vld);
  sc_out<sc_lv<type_info_dma_dbg_dat::width>> SC_NAMED(dma_dbg_dat);

  sc_signal<type_info_r_master0_ar_dat::data_type> SC_NAMED(sig_r_master0_ar_dat);
  sc_signal<type_info_r_master0_r_dat::data_type> SC_NAMED(sig_r_master0_r_dat);
  sc_signal<type_info_w_master0_aw_dat::data_type> SC_NAMED(sig_w_master0_aw_dat);
  sc_signal<type_info_w_master0_w_dat::data_type> SC_NAMED(sig_w_master0_w_dat);
  sc_signal<type_info_w_master0_b_dat::data_type> SC_NAMED(sig_w_master0_b_dat);
  sc_signal<type_info_r_slave0_ar_dat::data_type> SC_NAMED(sig_r_slave0_ar_dat);
  sc_signal<type_info_r_slave0_r_dat::data_type> SC_NAMED(sig_r_slave0_r_dat);
  sc_signal<type_info_w_slave0_aw_dat::data_type> SC_NAMED(sig_w_slave0_aw_dat);
  sc_signal<type_info_w_slave0_w_dat::data_type> SC_NAMED(sig_w_slave0_w_dat);
  sc_signal<type_info_w_slave0_b_dat::data_type> SC_NAMED(sig_w_slave0_b_dat);
  sc_signal<type_info_dma_done_dat::data_type> SC_NAMED(sig_dma_done_dat);
  sc_signal<type_info_dma_dbg_dat::data_type> SC_NAMED(sig_dma_dbg_dat);

  sc_clock connections_clk;
  sc_event check_event;

  virtual void start_of_simulation() {
    Connections::get_sim_clk().add_clock_alias(
      connections_clk.posedge_event(), clk.posedge_event());
  }

  SC_CTOR(dma_wrap) 
  : connections_clk("connections_clk", 10, SC_NS, 0.5,0,SC_NS,true)
  {
    SC_METHOD(check_clock);
    sensitive << connections_clk << clk;

    SC_METHOD(check_event_method);
    sensitive << check_event;

    trace_file_ptr = sc_create_vcd_trace_file("trace");
    trace_hierarchy(this, trace_file_ptr);

    SC_METHOD(method_r_master0_ar_dat); sensitive << sig_r_master0_ar_dat;
    SC_METHOD(method_r_master0_r_dat); sensitive << r_master0_r_dat; dont_initialize();
    SC_METHOD(method_w_master0_aw_dat); sensitive << sig_w_master0_aw_dat;
    SC_METHOD(method_w_master0_w_dat); sensitive << sig_w_master0_w_dat;
    SC_METHOD(method_w_master0_b_dat); sensitive << w_master0_b_dat; dont_initialize();
    SC_METHOD(method_r_slave0_ar_dat); sensitive << r_slave0_ar_dat; dont_initialize();
    SC_METHOD(method_r_slave0_r_dat); sensitive << sig_r_slave0_r_dat;
    SC_METHOD(method_w_slave0_aw_dat); sensitive << w_slave0_aw_dat; dont_initialize();
    SC_METHOD(method_w_slave0_w_dat); sensitive << w_slave0_w_dat; dont_initialize();
    SC_METHOD(method_w_slave0_b_dat); sensitive << sig_w_slave0_b_dat;
    SC_METHOD(method_dma_done_dat); sensitive << sig_dma_done_dat;
    SC_METHOD(method_dma_dbg_dat); sensitive << sig_dma_dbg_dat;

    dma_inst.clk(clk);
    dma_inst.rst_bar(rst_bar);
    dma_inst.r_master0.ar.rdy(r_master0_ar_rdy);
    dma_inst.r_master0.ar.vld(r_master0_ar_vld);
    dma_inst.r_master0.ar.dat(sig_r_master0_ar_dat);
    dma_inst.r_master0.r.rdy(r_master0_r_rdy);
    dma_inst.r_master0.r.vld(r_master0_r_vld);
    dma_inst.r_master0.r.dat(sig_r_master0_r_dat);
    dma_inst.w_master0.aw.rdy(w_master0_aw_rdy);
    dma_inst.w_master0.aw.vld(w_master0_aw_vld);
    dma_inst.w_master0.aw.dat(sig_w_master0_aw_dat);
    dma_inst.w_master0.w.rdy(w_master0_w_rdy);
    dma_inst.w_master0.w.vld(w_master0_w_vld);
    dma_inst.w_master0.w.dat(sig_w_master0_w_dat);
    dma_inst.w_master0.b.rdy(w_master0_b_rdy);
    dma_inst.w_master0.b.vld(w_master0_b_vld);
    dma_inst.w_master0.b.dat(sig_w_master0_b_dat);
    dma_inst.r_slave0.ar.rdy(r_slave0_ar_rdy);
    dma_inst.r_slave0.ar.vld(r_slave0_ar_vld);
    dma_inst.r_slave0.ar.dat(sig_r_slave0_ar_dat);
    dma_inst.r_slave0.r.rdy(r_slave0_r_rdy);
    dma_inst.r_slave0.r.vld(r_slave0_r_vld);
    dma_inst.r_slave0.r.dat(sig_r_slave0_r_dat);
    dma_inst.w_slave0.aw.rdy(w_slave0_aw_rdy);
    dma_inst.w_slave0.aw.vld(w_slave0_aw_vld);
    dma_inst.w_slave0.aw.dat(sig_w_slave0_aw_dat);
    dma_inst.w_slave0.w.rdy(w_slave0_w_rdy);
    dma_inst.w_slave0.w.vld(w_slave0_w_vld);
    dma_inst.w_slave0.w.dat(sig_w_slave0_w_dat);
    dma_inst.w_slave0.b.rdy(w_slave0_b_rdy);
    dma_inst.w_slave0.b.vld(w_slave0_b_vld);
    dma_inst.w_slave0.b.dat(sig_w_slave0_b_dat);
    dma_inst.dma_done.rdy(dma_done_rdy);
    dma_inst.dma_done.vld(dma_done_vld);
    dma_inst.dma_done.dat(sig_dma_done_dat);
    dma_inst.dma_dbg.rdy(dma_dbg_rdy);
    dma_inst.dma_dbg.vld(dma_dbg_vld);
    dma_inst.dma_dbg.dat(sig_dma_dbg_dat);
  }

  void check_clock() { check_event.notify(2, SC_PS);} // Let SC and Vlog delta cycles settle.

  void check_event_method() {
    if (connections_clk.read() == clk.read()) return;
    CCS_LOG("clocks misaligned!:"  << connections_clk.read() << " " << clk.read());
  }

  void method_r_master0_ar_dat(){
    typename type_info_r_master0_ar_dat::data_type obj;
    typename type_info_r_master0_ar_dat::sc_lv_type lv;
    obj = sig_r_master0_ar_dat.read();
    lv = TypeToBits(obj);
    r_master0_ar_dat = lv;
  }
  void method_r_master0_r_dat(){
    typename type_info_r_master0_r_dat::data_type obj;
    typename type_info_r_master0_r_dat::sc_lv_type lv;
    lv = r_master0_r_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    sig_r_master0_r_dat = obj;
  }
  void method_w_master0_aw_dat(){
    typename type_info_w_master0_aw_dat::data_type obj;
    typename type_info_w_master0_aw_dat::sc_lv_type lv;
    obj = sig_w_master0_aw_dat.read();
    lv = TypeToBits(obj);
    w_master0_aw_dat = lv;
  }
  void method_w_master0_w_dat(){
    typename type_info_w_master0_w_dat::data_type obj;
    typename type_info_w_master0_w_dat::sc_lv_type lv;
    obj = sig_w_master0_w_dat.read();
    lv = TypeToBits(obj);
    w_master0_w_dat = lv;
  }
  void method_w_master0_b_dat(){
    typename type_info_w_master0_b_dat::data_type obj;
    typename type_info_w_master0_b_dat::sc_lv_type lv;
    lv = w_master0_b_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    sig_w_master0_b_dat = obj;
  }
  void method_r_slave0_ar_dat(){
    typename type_info_r_slave0_ar_dat::data_type obj;
    typename type_info_r_slave0_ar_dat::sc_lv_type lv;
    lv = r_slave0_ar_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    sig_r_slave0_ar_dat = obj;
  }
  void method_r_slave0_r_dat(){
    typename type_info_r_slave0_r_dat::data_type obj;
    typename type_info_r_slave0_r_dat::sc_lv_type lv;
    obj = sig_r_slave0_r_dat.read();
    lv = TypeToBits(obj);
    r_slave0_r_dat = lv;
  }
  void method_w_slave0_aw_dat(){
    typename type_info_w_slave0_aw_dat::data_type obj;
    typename type_info_w_slave0_aw_dat::sc_lv_type lv;
    lv = w_slave0_aw_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    sig_w_slave0_aw_dat = obj;
  }
  void method_w_slave0_w_dat(){
    typename type_info_w_slave0_w_dat::data_type obj;
    typename type_info_w_slave0_w_dat::sc_lv_type lv;
    lv = w_slave0_w_dat.read();
    obj = BitsToType<decltype(obj)>(lv);
    sig_w_slave0_w_dat = obj;
  }
  void method_w_slave0_b_dat(){
    typename type_info_w_slave0_b_dat::data_type obj;
    typename type_info_w_slave0_b_dat::sc_lv_type lv;
    obj = sig_w_slave0_b_dat.read();
    lv = TypeToBits(obj);
    w_slave0_b_dat = lv;
  }
  void method_dma_done_dat(){
    typename type_info_dma_done_dat::data_type obj;
    typename type_info_dma_done_dat::sc_lv_type lv;
    obj = sig_dma_done_dat.read();
    lv = TypeToBits(obj);
    dma_done_dat = lv;
  }
  void method_dma_dbg_dat(){
    typename type_info_dma_dbg_dat::data_type obj;
    typename type_info_dma_dbg_dat::sc_lv_type lv;
    obj = sig_dma_dbg_dat.read();
    lv = TypeToBits(obj);
    dma_dbg_dat = lv;
  }

};

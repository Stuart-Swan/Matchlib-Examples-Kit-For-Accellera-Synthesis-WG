
#pragma once


#include "new_connections.h"

#ifdef __SYNTHESIS__
#define SYNTH_NAME(prefix, nm) ""
#else
#define SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif


template <class T>
struct dat_vld_chan : public sc_channel {
  dat_vld_chan(sc_module_name nm) : sc_channel(nm) {}

  sc_signal<bool> SC_NAMED(vld);
  sc_signal<T> SC_NAMED(dat);
};

template <class T>
struct dat_vld_in {
  dat_vld_in(const char* nm = "") :
    vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  {}

  sc_in<bool> vld;
  sc_in<T> dat;

  template <class C>
  void operator()(C& c) {
     vld(c.vld);
     dat(c.dat);
  }
};

template <class T>
struct dat_vld_out {
  dat_vld_out(const char* nm = "") :
    vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  {}

  sc_out<bool> vld;
  sc_out<T> dat;

  template <class C>
  void operator()(C& c) {
     vld(c.vld);
     dat(c.dat);
  }
};


template <class T>
class dat_vld_out_xactor : public sc_module
{
public:
  Connections::In <T> SC_NAMED(in1);

  dat_vld_out<T> SC_NAMED(out1);

  SC_CTOR(dat_vld_out_xactor) {
    SC_METHOD(drive_rdy);
    sensitive << in1.rdy;
    
    SC_METHOD(drive_dat_vld);
    dont_initialize();
    sensitive << in1.dat << in1.vld;

#ifndef __SYNTHESIS__
    in1.disable_spawn();
#endif
  }

  void drive_rdy() {
    in1.rdy = 1;
  }

  void drive_dat_vld() {
    out1.vld = in1.vld;
    out1.dat = in1.dat;
  }
};

template <class T>
class dat_vld_in_xactor : public sc_module
{
public:
  Connections::Out<T> SC_NAMED(out1);
  dat_vld_in<T> SC_NAMED(in1);

  SC_CTOR(dat_vld_in_xactor) {
    SC_METHOD(test_rdy);
    sensitive << out1.rdy;
    
    SC_METHOD(drive_out);
    dont_initialize();
    sensitive << in1.vld << in1.dat;

#ifndef __SYNTHESIS__
    out1.disable_spawn();
#endif
  }

  void test_rdy() {
#ifndef __SYNTHESIS__
    if (!out1.rdy.read() && (sc_time_stamp() > sc_time(100, SC_PS)))
    {
      SC_REPORT_ERROR("InFromDatVld-01", "rdy signal is false");
    }
#endif
  }

  void drive_out() {
    out1.vld = in1.vld;
    out1.dat = in1.dat;
  }
};


////////////////
// Convenience classes for instantiating transactor modules in user designs

template <class T>
struct dat_vld_in_xact {
  std::string prefix;

  dat_vld_in_xact(const char* nm = "") :
    prefix(nm)
  , vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  , xactor1(SYNTH_NAME(nm, "_xactor1"))
  , chan(SYNTH_NAME(prefix, "_chan"))
  {
    xactor1.in1(*this);
    xactor1.out1(chan);
  }

  sc_in<bool> vld;
  sc_in<T> dat;

  T Pop() { return chan.Pop(); }

  dat_vld_in_xactor<T> xactor1;
  Connections::Combinational<T> chan;
};

template <class T>
struct dat_vld_out_xact {
  std::string prefix;

  dat_vld_out_xact(const char* nm = "") :
    prefix(nm)
  , vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  , xactor1(SYNTH_NAME(nm, "_xactor1"))
  , chan(SYNTH_NAME(prefix, "_chan"))
  {
    xactor1.out1(*this);
    xactor1.in1(chan);
  }

  sc_out<bool> vld;
  sc_out<T> dat;

  void Push(const T& v) { chan.Push(v); }

  dat_vld_out_xactor<T> xactor1;
  Connections::Combinational<T> chan;
};


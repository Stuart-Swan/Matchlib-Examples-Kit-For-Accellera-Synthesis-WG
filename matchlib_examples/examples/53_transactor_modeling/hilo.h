
#pragma once

/*
This file shows 3 different ways to model transactors for the same underlying protocol.

The protocol used here is very simple but intended to illustrate common issues in modeling
real-world protocols that are not simple message passing protocols.

The protocol is called "hilo".

The "hilo" protocol is the same as the basic rdy/vld/dat protcol with the following differences:
 - The "dat" signal is an sc_uint<8>
 - When rdy & vld are both true, the high byte is transferred.
 - In the very next clock cycle, rdy and vld are driven low and the low byte is transferred.
 - Each call to the transactor thus transfers a sc_uint<16> message.

*/

#include "sc_named.h"

#ifdef __SYNTHESIS__
#define SYNTH_NAME(prefix, nm) ""
#else
#define SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif

////////////////////////////////////////////
//
// The hilo protocol channel. This channel is the same for all transactor styles

struct hilo_chan : public sc_channel {
  hilo_chan(sc_module_name nm) : sc_channel(nm) {}

  sc_signal<bool> SC_NAMED(rdy);
  sc_signal<bool> SC_NAMED(vld);
  sc_signal<sc_uint<8>> SC_NAMED(dat);
};

// The hilo protocol input port. This port is the same for all transactor styles

struct hilo_in {
  hilo_in(const char* nm = "") :
    vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  , rdy(SYNTH_NAME(nm, "_rdy"))
  {}

  sc_in<bool> vld;
  sc_in<sc_uint<8>> dat;
  sc_out<bool> rdy;

  template <class C>
  void operator()(C& c) {
     vld(c.vld);
     dat(c.dat);
     rdy(c.rdy);
  }
};

// The hilo protocol output port. This port is the same for all transactor styles

struct hilo_out {
  hilo_out(const char* nm = "") :
    vld(SYNTH_NAME(nm, "_vld"))
  , dat(SYNTH_NAME(nm, "_dat"))
  , rdy(SYNTH_NAME(nm, "_rdy"))
  {}

  sc_out<bool> vld;
  sc_out<sc_uint<8>> dat;
  sc_in<bool> rdy;

  template <class C>
  void operator()(C& c) {
     vld(c.vld);
     dat(c.dat);
     rdy(c.rdy);
  }
};

///////////////////
// Transactor Style 1: Simple functions to be inlined into caller

template <class C>
void hilo_reset_read(C& chan) {
  chan.rdy = 0;
}

template <class C>
sc_uint<16> hilo_pop(C& chan) {

  chan.rdy = 1;
  do {
   wait();
  } while (chan.vld.read() == 0);
  chan.rdy = 0;

  sc_uint<8> hi_byte = chan.dat.read();
  wait();
  sc_uint<8> lo_byte = chan.dat.read();
  return ((hi_byte << 8) | lo_byte);
}

template <class C>
void hilo_reset_write(C& chan) {
  chan.vld = 0;
  chan.dat = 0;
}

template <class C>
void hilo_push(C& chan, sc_uint<16> v) {

  chan.vld = 1;
  chan.dat = v >> 8;
  do {
   wait();
  } while (chan.rdy.read() == 0);
  chan.vld = 0;

  chan.dat = v;
  wait();
}

///////////////////
// Transactor Style 2: Transactors using "pragma modulario" functions

#pragma design modulario <out> 
template <class C>
void hilo_push_mio(C& chan, sc_uint<16> v) {

  bool do_lo = 0;

  chan.vld = 1;
  chan.dat = v >> 8;

  while (1) {
   if (do_lo)
     chan.dat = v;

   wait();  // pragma modulario needs a single wait statement

   if (do_lo)
     break;

   if (chan.rdy.read() == 1)
   {
     chan.vld = 0;
     do_lo = 1;
   }
  }
}

#pragma design modulario <in> 
template <class C>
sc_uint<16> hilo_pop_mio(C& chan) {

  bool do_lo = 0;
  sc_uint<8> hi_byte = 0;
  sc_uint<8> lo_byte = 0;

  chan.rdy = 1;
  while (1) {
    wait(); // pragma modulario requires a single wait statement

    if (do_lo) {
      lo_byte = chan.dat.read();
      break;
    }

    if (chan.vld.read() == 1) {
      chan.rdy = 0;
      hi_byte = chan.dat.read();
      do_lo = 1;
    }
  }

  return ((hi_byte << 8) | lo_byte);
}

//////////////////////
// Transactor Style 3: Transactors modeled as separate sc_modules and SC_THREADs

struct hilo_in_xactor : public sc_module {
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  hilo_in SC_NAMED(hilo_in1);
  Connections::Out<sc_uint<16>> SC_NAMED(chan);

  SC_CTOR(hilo_in_xactor) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifdef CONNECTIONS_SIM_ONLY
    chan.disable_spawn();
#endif
  }

#pragma implicit_fsm true
  void main() {
    chan.vld = 0;
    chan.dat = 0;
    hilo_in1.rdy = 0;
    wait();

    bool got_hi_byte = 0;
    bool got_lo_byte = 0;
    bool get_lo_byte = 0;
    bool got_word = 0;
    sc_uint<16> word;
    sc_uint<8> hi_byte;
    sc_uint<8> lo_byte;

    // This is "RTL in SystemC". 

    while (1) {
      if (!got_hi_byte) {
        hilo_in1.rdy = 1;
      }
      if (got_word) {
        chan.vld = 1;
        chan.dat = word;
      }
      wait();
      if (got_word && (chan.rdy.read() == 1)) {
        got_word = 0;
        chan.vld = 0;
      }
      if (get_lo_byte) {
        lo_byte = hilo_in1.dat.read();
        get_lo_byte = 0;
        got_lo_byte = 1;
      }
      if (!got_hi_byte && (hilo_in1.vld.read() == 1)) {
        hilo_in1.rdy = 0;
        hi_byte = hilo_in1.dat.read();
        got_hi_byte = 1;
        get_lo_byte = 1;
      }
      if (got_hi_byte && got_lo_byte) {
        got_hi_byte = 0;
        got_lo_byte = 0;
        word = (hi_byte << 8) | lo_byte;
        got_word = 1;
      }
    }
   }
};

struct hilo_out_xactor : public sc_module {
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  hilo_out SC_NAMED(hilo_out1);
  Connections::In<sc_uint<16>> SC_NAMED(chan);

  SC_CTOR(hilo_out_xactor) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifdef CONNECTIONS_SIM_ONLY
    chan.disable_spawn();
#endif
  }

#pragma implicit_fsm true
  void main() {
    chan.rdy = 0;
    hilo_out1.vld = 0;
    hilo_out1.dat = 0;
    wait();

    // This is "RTL in SystemC". 

    chan.rdy = 1;
    do {
     wait();
    } while (chan.vld.read() == 0);
    chan.rdy = 0;
    sc_uint<16> v = chan.dat.read();

    while (1) {
      hilo_out1.vld = 1;
      hilo_out1.dat = v >> 8;
      do {
        wait();
      } while (hilo_out1.rdy.read() == 0);
      hilo_out1.vld = 0;
      hilo_out1.dat = v;
      chan.rdy = 1;
      do {
       wait();
      } while (chan.vld.read() == 0);
      chan.rdy = 0;
      v = chan.dat.read();
    }
  }
};

////////////////
// Convenience classes for instantiating transactor modules in user designs

template <class T>
struct hilo_in_xact {
  std::string prefix;
  hilo_in_xactor in_xactor1;
  Connections::Combinational<T> chan;

  hilo_in_xact(const char* s) : 
    prefix(s)
  , in_xactor1(SYNTH_NAME(prefix, "_xactor1"))
  , chan(SYNTH_NAME(prefix, "_chan"))
  {
  }

  template <class C, class R, class P>
  void bind(C& clk, R& rst_bar, P& in1) {
    in_xactor1.clk(clk);
    in_xactor1.rst_bar(rst_bar);
    in_xactor1.chan(chan);
    in_xactor1.hilo_in1(in1);
  }
};

template <class T>
struct hilo_out_xact {
  std::string prefix;
  hilo_out_xactor out_xactor1;
  Connections::Combinational<T> chan;

  hilo_out_xact(const char* s) : 
    prefix(s)
  , out_xactor1(SYNTH_NAME(prefix, "_xactor1"))
  , chan(SYNTH_NAME(prefix, "_chan"))
  {
  }

  template <class C, class R, class P>
  void bind(C& clk, R& rst_bar, P& out1) {
    out_xactor1.clk(clk);
    out_xactor1.rst_bar(rst_bar);
    out_xactor1.chan(chan);
    out_xactor1.hilo_out1(out1);
  }
};


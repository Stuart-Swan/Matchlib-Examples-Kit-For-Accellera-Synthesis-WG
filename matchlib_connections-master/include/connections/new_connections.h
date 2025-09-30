
/*
new_connections.h
Stuart Swan, Platform Architect, Siemens EDA
29 Sept 2025

This is a complete rewrite of the old connections.h file.
Goals:
 - "Drop in" replacement for old connections.h file.
 - Cleanup & major simplification
 - Removal of obsolete features
 - pre-HLS sim performance improvement for DIRECT_PORT

Removed Features from old connections.h:
 - MARSHALL_PORT, SYN_PORT - use DIRECT_PORT instead
 - rdy/val/msg signal naming - use rdy/vld/dat instead
 - overloaded Connections Port binding operators to allow mismatched port types to be bound.

Features still to be implemented from old
 - Error checking in pre-HLS sim.
 - TLM_PORT, FAST_SIM
 - random stall injection
 - latency and capacity backannotation
*/



#pragma once

#ifndef __SYNTHESIS__
#define CONNECTIONS_SIM_ONLY
#endif

#include <systemc.h>
#if defined(CONNECTIONS_SIM_ONLY) && !defined(SC_INCLUDE_DYNAMIC_PROCESSES)
#if !defined(NC_SYSTEMC) && !defined(XM_SYSTEMC)
#warning "SC_INCLUDE_DYNAMIC_PROCESSES is being defined and reentrant <systemc> header included"
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#else
#error "Make sure SC_INCLUDE_DYNAMIC_PROCESSES is defined BEFORE first systemc.h include"
#endif
#endif

#include "marshaller.h"
#include "connections_utils.h"
#include "connections_trace.h"
#include "message.h"

#ifdef CONNECTIONS_SIM_ONLY
#include <iomanip>
#include <vector>
#include <map>
#include <type_traits>
#include <tlm.h>
#if !defined(NC_SYSTEMC) && !defined(XM_SYSTEMC) && !defined(NO_SC_RESET_INCLUDE)
#include <sysc/kernel/sc_reset.h>
#define HAS_SC_RESET_API
#endif
#endif

#define _VLDNAME_ vld
#define _RDYNAME_ rdy
#define _DATNAME_ dat
#define _VLDNAMESTR_ "vld"
#define _RDYNAMESTR_ "rdy"
#define _DATNAMESTR_ "dat"


#define AUTO_PORT Connections::DIRECT_PORT

#ifdef __SYNTHESIS__
#define SYNTH_NAME(prefix, nm) ""
#else
#define SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif


namespace Connections {

enum connections_port_t {SYN_PORT = 0, MARSHALL_PORT = 1, DIRECT_PORT = 2, TLM_PORT=3};

struct chan_base
#ifndef __SYNTHESIS__
 : public sc_channel
#endif
{
  chan_base(sc_module_name nm) 
#ifndef __SYNTHESIS__
   : sc_channel(nm) 
#endif
  {}
};

template <typename Message> 
class In_if : public sc_interface {
public:
  virtual Message Pop() = 0;
  virtual bool PopNB(Message& m) = 0;
  virtual void ResetRead() = 0;
  virtual void disable_spawn_in() = 0;
};

template <typename Message> 
class Out_if : public sc_interface {
public:
  virtual void Push(const Message& m) = 0;
  virtual bool PushNB(const Message& m) = 0;
  virtual void ResetWrite() = 0;
  virtual void disable_spawn_out() = 0;
};


template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Combinational 
  : public chan_base 
  , public In_if<Message>
  , public Out_if<Message> 
{
public:
#ifndef __SYNTHESIS__
  SC_HAS_PROCESS(Combinational);
#endif

  Combinational(sc_module_name nm) 
   : chan_base(nm) 
   , vld("vld")
   , dat("dat")
   , rdy("rdy")
  {
#ifndef __SYNTHESIS__
    SC_THREAD(pre_thread);
    SC_THREAD(post_thread);
#endif
  }

  Combinational() : chan_base(sc_module_name(sc_gen_unique_name("Comb"))) {
    // This ctor needed for legacy code to compile, but cannot explicitly name vld/dat/rdy
    // else will cause naming warnings at pre-hls sim elaboration time.
#ifndef __SYNTHESIS__
    SC_THREAD(pre_thread);
    SC_THREAD(post_thread);
#endif
  } 

#ifdef __SYNTHESIS__

  void pre_thread() {}
  void post_thread() {}
  void disable_spawn_in() {}
  void disable_spawn_out() {}

  void ResetWrite() {
   vld = 0;
   dat = Message();
  }

  void ResetRead() {
   rdy = 0;
  }

#pragma builtin_modulario
#pragma design modulario < in >
  Message Pop() {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
    return dat.read();
  }

#pragma builtin_modulario
#pragma design modulario < in >
  bool PopNB(Message& m) { 
   rdy = 1;
   wait();
   rdy = 0;
   m = dat;
   return vld;
  }

#pragma builtin_modulario
#pragma design modulario < out >
  void Push(const Message& m) {
    do {
      vld.write(true); 
      dat.write(m);
      wait();
    } while (rdy.read() != true);
    vld.write(false); 
  }

#pragma builtin_modulario
#pragma design modulario < out >
  bool PushNB(const Message& m) {
   vld = 1;
   dat = m;
   wait();
   vld = 0;
   return rdy;
  }

#else

  Message in_buf_dat;
  bool    in_buf_vld;
  Message out_buf_dat;
  bool    out_buf_vld;

  void ResetWrite() {
   out_buf_vld = 0;
   out_buf_dat = Message();
  }

  void ResetRead() {
   in_buf_vld = 0;
  }

  Message Pop() {
   if (!in_buf_vld) {
     do {
      wait();
     } while (!in_buf_vld);
   }

   in_buf_vld = 0;
   return in_buf_dat;
  }

  bool PopNB(Message& m) { 
   if (in_buf_vld) {
    m = in_buf_dat;
    in_buf_vld = 0;
    return 1;
   }

   return 0;
  }

  void Push(const Message& m) {
   if (out_buf_vld) {
     do {
      wait();
     } while (out_buf_vld);
   }

   out_buf_vld = 1;
   out_buf_dat = m;
  }

  bool PushNB(const Message& m) {
   if (!out_buf_vld) {
     out_buf_vld = 1;
     out_buf_dat = m;
     return 1;
   } else {
     return 0;
   }
  }

  void Pre() {
   // In:
   if (!disable_in && !in_buf_vld && vld) {
    in_buf_vld = 1;
    in_buf_dat = dat;
   }

   // Out:
   if (!disable_out && vld && rdy)
    out_buf_vld = 0;
  }

  void Post() {
   // In:
   if (!disable_in) {
     if (!in_buf_vld)
      rdy = 1;
     else
      rdy = 0;
   }

   // Out:
   if (!disable_out) {
     if (out_buf_vld) {
      vld = 1;
      dat = out_buf_dat;
     } else {
      vld = 0;
     }
   }
  }

  void pre_thread() {
    wait(0.9, SC_NS);

    while (1) {
      Pre();
      wait(1.0, SC_NS);
    }
  }

  void post_thread() {
    wait(1.1, SC_NS);

    while (1) {
      Post();
      wait(1.0, SC_NS);
    }
  }

  bool disable_in{0};
  bool disable_out{0};

  void disable_spawn_in() {
    disable_in=1;
  }

  void disable_spawn_out() {
    disable_out=1;
  }
  
#endif

  sc_signal<bool> vld;
  sc_signal<Message> dat;
  sc_signal<bool> rdy;
};


template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class In 
  : public sc_port<In_if<Message>> 
{
public:
  typedef sc_port<In_if<Message>> port_t;

  In(const char* nm = sc_gen_unique_name("In")) : port_t(nm) {}

#ifdef __SYNTHESIS__

#pragma builtin_modulario
#pragma design modulario < in >
  Message Pop() {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
    return dat.read();
  }

#pragma builtin_modulario
#pragma design modulario < in >
  bool PopNB(Message& m) { 
   rdy = 1;
   wait();
   rdy = 0;
   m = dat;
   return vld;
  }

  void Reset() {
    rdy = 0;
  }

  void disable_spawn() {}

#else

  Message Pop() { port_t* p=this; return (*p)->Pop(); }

  bool PopNB(Message& m) { port_t* p=this; return (*p)->PopNB(m); }

  void Reset() { port_t* p=this; (*p)->ResetRead(); }

  bool do_disable{0};

  void disable_spawn() {do_disable=1;}

  void end_of_elaboration() {
   if (do_disable) {
    port_t* p=this; 
    (*p)->disable_spawn_in(); 
   }
  }

#endif

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
#ifndef __SYNTHESIS__
   port_t* p = this;
   (*p)(rhs);
#endif
  }

  sc_in<bool> vld{SYNTH_NAME(this->basename(), "vld")};
  sc_out<bool> rdy{SYNTH_NAME(this->basename(), "rdy")};
  sc_in<Message> dat{SYNTH_NAME(this->basename(), "dat")};
};

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Out 
  : public sc_port<Out_if<Message>> 
{
public:
  typedef sc_port<Out_if<Message>> port_t;

  Out(const char* nm = sc_gen_unique_name("Out")) : port_t(nm) {}

#ifdef __SYNTHESIS__

#pragma builtin_modulario
#pragma design modulario < out >
  void Push(const Message& m) {
    do {
      vld.write(true); 
      dat.write(m);
      wait();
    } while (rdy.read() != true);
    vld.write(false); 
  }

#pragma builtin_modulario
#pragma design modulario < out >
  bool PushNB(const Message& m) {
   vld = 1;
   dat = m;
   wait();
   vld = 0;
   return rdy;
  }

  void Reset() {
   vld = 0;
   dat = Message();
  }

  void disable_spawn() {}

#else

  void Push(const Message& m) { port_t* p = this; (*p)->Push(m); }

  bool PushNB(const Message& m) { port_t* p=this; return (*p)->PushNB(m); }

  void Reset() { port_t* p=this; (*p)->ResetWrite(); }

  bool do_disable{0};

  void disable_spawn() {do_disable=1;}

  void end_of_elaboration() {
   if (do_disable) {
    port_t* p=this; 
    (*p)->disable_spawn_out(); 
   }
  }

#endif

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
#ifndef __SYNTHESIS__
   sc_port<Out_if<Message>>* p = this;
   (*p)(rhs);
#endif
  }

  sc_out<bool> vld{SYNTH_NAME(this->basename(), "vld")};
  sc_in<bool> rdy{SYNTH_NAME(this->basename(), "rdy")};
  sc_out<Message> dat{SYNTH_NAME(this->basename(), "dat")};
};

// Forward declarations below needed for other Matchlib headers currently

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class InBlocking;
template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class OutBlocking;

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Bypass;
template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Pipeline;
template <typename Message, unsigned int NumEntries, connections_port_t port_marshall_type = AUTO_PORT>
class Buffer;


} // namespace Connections

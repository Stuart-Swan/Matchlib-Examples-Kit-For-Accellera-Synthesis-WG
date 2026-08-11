
#pragma once

#include <systemc.h>

namespace sc_hls { namespace msg_lib {

struct catapult_view_tag {};

#define SC_HLS_DEFAULT_SYNTH_VIEW_TAG catapult_view_tag;

} }

#include "sc_hls_msg.h"
#include "sc_hls_sync.h"

namespace sc_hls { namespace msg_lib {


template <typename Message>
class msg_channel<Message, catapult_view_tag>
{
public:
    sc_signal<bool>     vld;
    sc_signal<bool>     rdy;
    sc_signal<Message>  dat;

    msg_channel(const sc_module_name &nm) {}

    msg_channel() {}

  void reset_push() {
   vld = 0;
   dat = Message();
  }
  
  void reset_pop() {
   rdy = 0;
  }

#pragma builtin_modulario
#pragma design modulario < in >
  Message pop() {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
    return dat.read();
  }


#pragma builtin_modulario
#pragma design modulario < in >
  bool pop_nb(Message& m) {
   rdy = 1;
   wait();
   rdy = 0;
   m = dat;
   return vld;
  }

#pragma builtin_modulario
#pragma design modulario < out >
  void push(const Message& m) {
    do {
      vld.write(true);
      dat.write(m);
      wait();
    } while (rdy.read() != true);
    vld.write(false);
  }

#pragma builtin_modulario
#pragma design modulario < out >
  bool push_nb(const Message& m) {
   vld = 1;
   dat = m;
   wait();
   vld = 0;
   return rdy;
  }
};

template <typename Message>
class msg_in<Message, catapult_view_tag>
{
public:
  msg_in(const char* nm = sc_gen_unique_name("msg_in")) {}

#pragma builtin_modulario
#pragma design modulario < in >
  Message pop() {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
    return dat.read();
  }

#pragma builtin_modulario
#pragma design modulario < in >
  bool pop_nb(Message& m) {
   rdy = 1;
   wait();
   rdy = 0;
   m = dat;
   return vld;
  }

  void reset_pop() {
    rdy = 0;
  }

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
  }

  template <typename M>
  void operator()(msg_in<M> &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
  }

  sc_in<bool> vld;
  sc_out<bool> rdy;
  sc_in<Message> dat;
};

template <typename Message>
class msg_out<Message, catapult_view_tag>
{
public:
  msg_out(const char* nm = sc_gen_unique_name("msg_out")) {}

#pragma builtin_modulario
#pragma design modulario < out >
  void push(const Message& m) {
    do {
      vld.write(true);
      dat.write(m);
      wait();
    } while (rdy.read() != true);
    vld.write(false);
  }

#pragma builtin_modulario
#pragma design modulario < out >
  bool push_nb(const Message& m) {
   vld = 1;
   dat = m;
   wait();
   vld = 0;
   return rdy;
  }

  void reset_push() {
   vld = 0;
   dat = Message();
  }

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
  }

  template <typename M>
  void operator()(msg_out<M> &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
  }

  sc_out<bool> vld;
  sc_in<bool> rdy;
  sc_out<Message> dat;
};


template <>
class sync_channel <catapult_view_tag> {
public:
  sync_channel(sc_module_name nm)  {}

  sc_signal<bool> SC_NAMED(rdy);
  sc_signal<bool> SC_NAMED(vld);

  #pragma builtin_modulario
  #pragma design modulario<sync>
  template <typename ...T>
  void sync_read(T &...t) {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true );
    rdy.write(false);
  }

  #pragma builtin_modulario
  #pragma design modulario<sync>
  template <typename ...T>
  void sync_write(T &...t) {
    do {
      vld.write(true);
      wait();
    } while (rdy.read() != true );
    vld.write(false);
  }

  void reset_pop() {
    rdy = 0;
  }

  void reset_push() {
    vld = 0;
  }
};

template <>
class sync_in<catapult_view_tag>
{
public:
  sync_in(const char* nm = "") {}

#pragma builtin_modulario
#pragma design modulario < sync >
  template <typename ...T>
  void sync(T &...t) {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
  }

  void reset_pop() {
    rdy = 0;
  }

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
  }

  sc_in<bool> vld;
  sc_out<bool> rdy;
};

template <>
class sync_out<catapult_view_tag>
{
public:
  sync_out(const char* nm = "") {}

#pragma builtin_modulario
#pragma design modulario < sync >
  template <typename ...T>
  void sync(T &...t) {
    do {
      vld.write(true);
      wait();
    } while (rdy.read() != true);
    vld.write(false);
  }

  void reset_push() {
    vld = 0;
  }

  template <typename C>
  void operator()(C &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
  }

  sc_out<bool> vld;
  sc_in<bool> rdy;
};

} }


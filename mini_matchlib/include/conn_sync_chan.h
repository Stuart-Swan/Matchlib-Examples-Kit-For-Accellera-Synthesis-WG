
#pragma once

#include "new_connections.h"

namespace Connections {

class SyncChannel : public chan_base {
public:
  SyncChannel(sc_module_name nm) : chan_base(nm) {}

  sc_signal<bool> SC_NAMED(rdy);
  sc_signal<bool> SC_NAMED(vld);

  #pragma builtin_modulario
  #pragma design modulario<sync>
  template <typename ...T>
  void sync_in(T &...t) {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true );
    rdy.write(false);
  }

  #pragma builtin_modulario
  #pragma design modulario<sync>
  template <typename ...T>
  void sync_out(T &...t) {
    do {
      vld.write(true);
      wait();
    } while (rdy.read() != true );
    vld.write(false);
  }

  void ResetRead() {
    rdy = 0;
  }

  void ResetWrite() {
    vld = 0;
  }
};

class SyncIn 
{
public:
  SyncIn(const char* nm = sc_gen_unique_name("SyncIn")) {}

#pragma builtin_modulario
#pragma design modulario < sync >
  template <typename ...T>
  void sync_in(T &...t) {
    do {
      rdy.write(true);
      wait();
    } while (vld.read() != true);
    rdy.write(false);
  }

  void Reset() {
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

class SyncOut
{
public:
  SyncOut(const char* nm = sc_gen_unique_name("SyncOut")) {}

#pragma builtin_modulario
#pragma design modulario < sync >
  template <typename ...T>
  void sync_out(T &...t) {
    do {
      vld.write(true);
      wait();
    } while (rdy.read() != true);
    vld.write(false);
  }

  void Reset() {
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

} // namespace Connections

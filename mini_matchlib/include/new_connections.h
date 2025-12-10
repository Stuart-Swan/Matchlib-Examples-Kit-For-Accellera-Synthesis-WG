
/*
new_connections.h mini version

Stuart Swan, Platform Architect, Siemens EDA
24 Oct 2025

This is a minimalist version of the standard new_connections.h file
for use in standardization discussions and also for educational use.

This version is completely self-contained and only has dependencies
on a standard SystemC installation (via $SYSTEMC_HOME), and
has no dependencies on any other NVidia or Siemens EDA library files.


Limitations:
 - Only handles a single global clock , which must be explicitly set by user via set_sim_clk
 - For clarity, features such as logging, rand stall, tracing, back annotation have been removed 
 - See normal new_connections.h for fully implemented set of features.
*/



#pragma once

#include <systemc.h>
#if !defined(SC_INCLUDE_DYNAMIC_PROCESSES)
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#else
#error "Make sure SC_INCLUDE_DYNAMIC_PROCESSES is defined BEFORE first systemc.h include"
#endif



#ifndef __SYNTHESIS__
#include <iomanip>
#include <vector>
#include <map>
#include <memory>
#include <type_traits>
#include <tlm.h>
#endif


#ifdef __SYNTHESIS__
#define CONN_SYNTH_NAME(prefix, nm) ""
#else
#define CONN_SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif


namespace Connections {

  class SimConnectionsClk;
  SimConnectionsClk &get_sim_clk();
  void set_sim_clk(sc_clock *clk_ptr);

  class ConManager;
  ConManager &get_conManager();


#ifndef __SYNTHESIS__

  class SimConnectionsClk
  {
  public:
    SimConnectionsClk() { }

    void set(sc_clock *clk_ptr_) {
      clock_ptr = clk_ptr_;
    }

    void start_of_simulation() {
      // Set epsilon to default value at start of simulation, after time resolution has been set
      epsilon = sc_time(0.01, SC_NS);
    }

    sc_core::sc_time epsilon;
    sc_clock* clock_ptr{0};
  };


  class Blocking_abs
  {
  public:
    Blocking_abs() { };
    virtual void Post() {};
    virtual void Pre()  {};
    virtual void PrePostReset()  {};
    bool disabled{0};
    virtual std::string base_name() {return "unnamed"; }
    virtual std::string full_name() { return "unnamed"; }
  };


  class ConManager
  {
  public:
    ConManager() { }

    std::vector<Blocking_abs *> tracked;

    void add(Blocking_abs *c) {
      tracked.push_back(c);
    }

    bool sim_clk_initialized{0};

    void init_sim_clk() {
      if (sim_clk_initialized) { return; }

      sim_clk_initialized = true;

      get_sim_clk().start_of_simulation();

      if (!get_sim_clk().clock_ptr) {
        SC_REPORT_ERROR("CONNECTIONS-99", "You must call Connections::set_sim_clk() before start of simulation");
        exit(-1);
      }

      sc_spawn(sc_bind(&ConManager::run, this), "run");
    }


    void add_clock_event(Blocking_abs *c) {
      init_sim_clk();
    }

    void run() {
      sc_time epsilon = get_sim_clk().epsilon;
      sc_time period = get_sim_clk().clock_ptr->period();
      sc_time post2pre_delay = period - (2 * epsilon);
      sc_time pre2post_delay = (2 * epsilon);

      wait(epsilon);

      while (1) {
        // Post();
        for (std::vector<Blocking_abs *>::iterator it=tracked.begin(); it!=tracked.end(); ) {
          (*it)->Post();
          ++it;
        }

        wait(post2pre_delay);

        //Pre();
        for (std::vector<Blocking_abs *>::iterator it=tracked.begin(); it!=tracked.end(); ) {
          (*it)->Pre();
          ++it;
        }

        wait(pre2post_delay);
      }
    }
  };

// See: https://stackoverflow.com/questions/18860895/how-to-initialize-static-members-in-the-header
  template <class Dummy>
  struct ConManager_statics {
    static SimConnectionsClk sim_clk;
    static ConManager conManager;
  };

  inline void set_sim_clk(sc_clock *clk_ptr)
  {
    ConManager_statics<void>::sim_clk.set(clk_ptr);
  }

  template <class Dummy>
  SimConnectionsClk ConManager_statics<Dummy>::sim_clk;
  template <class Dummy>
  ConManager ConManager_statics<Dummy>::conManager;

  inline SimConnectionsClk &get_sim_clk()
  {
    return ConManager_statics<void>::sim_clk;
  }

  inline ConManager &get_conManager()
  {
    return ConManager_statics<void>::conManager;
  }

#endif // !__SYNTHESIS__

#ifndef __SYNTHESIS__
struct chan_base : public sc_channel {
  chan_base(sc_module_name nm) : sc_channel(nm) {}
};
#else
struct chan_base {
  chan_base(sc_module_name nm) {}
};
#endif

template <typename Message> 
class In_if : public sc_interface {
public:
  virtual Message Pop() = 0;
  virtual bool PopNB(Message& m) = 0;
  virtual bool PeekNB(Message& m) = 0;
  virtual void ResetRead() = 0;
  virtual void disable_spawn_in() = 0;
  virtual void set_in_port_names(std::string full_name, std::string base_name) = 0;
};

template <typename Message> 
class Out_if : public sc_interface {
public:
  virtual void Push(const Message& m) = 0;
  virtual bool PushNB(const Message& m) = 0;
  virtual void ResetWrite() = 0;
  virtual void disable_spawn_out() = 0;
  virtual void set_out_port_names(std::string full_name, std::string base_name) = 0;
};

#ifndef __SYNTHESIS__

template <typename Message>
struct In_sim_port : public Blocking_abs {
  Message& in_buf_dat;
  bool&    in_buf_vld;
  sc_signal<bool>& rdy;
  sc_signal<bool>& vld;
  sc_signal<Message>& dat;

  In_sim_port(Message& _in_buf_dat, bool& _in_buf_vld, 
              sc_signal<bool>& _rdy, sc_signal<bool>& _vld, sc_signal<Message>& _dat)
    : in_buf_dat(_in_buf_dat)
    , in_buf_vld(_in_buf_vld)
    , rdy(_rdy)
    , vld(_vld)
    , dat(_dat)
  {
       get_conManager().add(this);
  }

public:
 
  void disable() { disabled = 1; }

  virtual void Post() {
   if (!disabled) {
     if (!in_buf_vld)
      rdy = 1;
     else
      rdy = 0;
   }
  }

  virtual void Pre() {
   if (!disabled && !in_buf_vld && vld) {
    in_buf_vld = 1;
    in_buf_dat = dat;
   }
  }

  virtual void PrePostReset() {
    in_buf_vld = 0;
  }

  std::string _base_name{"unnamed"};
  std::string _full_name{"unnamed"};

  virtual std::string base_name() {return _base_name; }
  virtual std::string full_name() { return _full_name; }
};


template <typename Message>
struct Out_sim_port : public Blocking_abs {
  Message& out_buf_dat;
  bool&    out_buf_vld;
  sc_signal<bool>& rdy;
  sc_signal<bool>& vld;
  sc_signal<Message>& dat;

  Out_sim_port(Message& _out_buf_dat, bool& _out_buf_vld, 
              sc_signal<bool>& _rdy, sc_signal<bool>& _vld, sc_signal<Message>& _dat)
    : out_buf_dat(_out_buf_dat)
    , out_buf_vld(_out_buf_vld)
    , rdy(_rdy)
    , vld(_vld)
    , dat(_dat)
  {
       get_conManager().add(this);
  }

  void disable() { disabled = 1; }

  virtual void Post() {
   if (!disabled) {
     if (out_buf_vld) {
      vld = 1;
      dat = out_buf_dat;
     } else {
      vld = 0;
     }
   }
  }

  virtual void Pre() {
   if (!disabled && vld && rdy)
    out_buf_vld = 0;
  }

  virtual void PrePostReset() {
    out_buf_vld = 0;
  }

  std::string _base_name{"unnamed"};
  std::string _full_name{"unnamed"};

  virtual std::string base_name() {return _base_name; }
  virtual std::string full_name() { return _full_name; }
};

#endif

template <typename Message>
class Combinational_base
  : public chan_base 
  , public In_if<Message>
  , public Out_if<Message> 
{
public:

  Combinational_base(sc_module_name nm) 
   : chan_base(nm) 
   , vld("vld")
   , dat("dat")
   , rdy("rdy")
  {
    Init();
  }


#ifdef __SYNTHESIS__

  void Init() {}
  void disable_spawn_in() {}
  void disable_spawn_out() {}
  virtual void set_in_port_names(std::string full_name, std::string base_name) {}
  virtual void set_out_port_names(std::string full_name, std::string base_name) {}

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
#pragma design modulario < peek >
  bool PeekNB(Message &data) {
   data = dat;
#ifdef __SYNTHESIS__
   rdy = 0;
#endif
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

  In_sim_port<Message> in_port{in_buf_dat, in_buf_vld, rdy, vld, dat};
  Out_sim_port<Message> out_port{out_buf_dat, out_buf_vld, rdy, vld, dat};

  void ResetWrite() {
   get_conManager().add_clock_event(&out_port);
   out_buf_vld = 0;
   Message zero{}; // default initializes anything..
   out_buf_dat = zero;
  }

  void ResetRead() {
   get_conManager().add_clock_event(&in_port);
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

  bool PeekNB(Message& m) { 
   if (in_buf_vld) {
    m = in_buf_dat;
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

  void check_disable_spawn() {
    if (sc_start_of_simulation_invoked()) {
     SC_REPORT_ERROR("CONNECTIONS-317", "Cannot call disable_spawn() after simulation is started");
     sc_stop();
    }
  }

  void disable_spawn_in() {
    check_disable_spawn();
    in_port.disable();
  }

  void disable_spawn_out() {
    check_disable_spawn();
    out_port.disable();
  }

  bool portless_channel_access_in = 1;
  bool portless_channel_access_out = 1;

  void set_in_port_names(std::string full_name, std::string base_name) {
    in_port._full_name = full_name;
    in_port._base_name = base_name;
    portless_channel_access_in = 0;
  }
  
  void set_out_port_names(std::string full_name, std::string base_name) {
    out_port._full_name = full_name;
    out_port._base_name = base_name;
    portless_channel_access_out = 0;
  }

  void start_of_simulation() {
    // at end_of_elaboration, if this channel has real user ports, then set_in/out_port_names was called.
    // At start_of_sim, if we see that set_in/out_port_names was not called,
    // then we have "port-less channel access"
    if (portless_channel_access_in) {
      in_port._full_name = std::string(name()) + ".In";
      in_port._base_name = ".In";
    }
    if (portless_channel_access_out) {
      out_port._full_name = std::string(name()) + ".Out";
      out_port._base_name = ".Out";
    }
  }

  void Init() { }

#endif

  sc_signal<bool> vld;
  sc_signal<Message> dat;
  sc_signal<bool> rdy;
};


template <typename Message>
class Combinational
: public Combinational_base<Message> 
{
public:
  Combinational(sc_module_name nm) : Combinational_base<Message>(nm) {}

  Combinational() : Combinational_base<Message>() {}
};


template <typename Message>
class In 
  : public sc_port<In_if<Message>> 
{
public:
  typedef sc_port<In_if<Message>> port_t;

  In(const char* nm = sc_gen_unique_name("In")) : port_t(nm) {
  }


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

#pragma builtin_modulario
#pragma design modulario < peek >
  bool PeekNB(Message &data) {
   m = dat;
#ifdef __SYNTHESIS__
   rdy = 0;
#endif
   return vld;
  }

  void Reset() {
    rdy = 0;
  }

  void disable_spawn() {}

#else

  Message Pop() { port_t* p=this; return (*p)->Pop(); }

  bool PopNB(Message& m) { port_t* p=this; return (*p)->PopNB(m); }

  bool PeekNB(Message& m) { port_t* p=this; return (*p)->PeekNB(m); }

  void Reset() { 
      port_t* p=this; (*p)->ResetRead(); 
  }

  bool do_disable{0};

  void disable_spawn() {
    do_disable=1;
    if (sc_start_of_simulation_invoked()) {
     SC_REPORT_ERROR("CONNECTIONS-317", "Cannot call disable_spawn() after simulation is started");
     sc_stop();
    }
  }

  void end_of_elaboration() {
   port_t* p=this; (*p)->set_in_port_names(this->name(), this->basename());

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

  template <typename M>
  void operator()(In<M> &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
#ifndef __SYNTHESIS__
   port_t* p = this;
   (*p)(rhs);
#endif
  }
 
  sc_in<bool> vld{CONN_SYNTH_NAME(this->basename(), "vld")};
  sc_out<bool> rdy{CONN_SYNTH_NAME(this->basename(), "rdy")};
  sc_in<Message> dat{CONN_SYNTH_NAME(this->basename(), "dat")};
};

template <typename Message>
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

  void Reset() { 
      port_t* p=this; (*p)->ResetWrite(); 
  }

  bool do_disable{0};

  void disable_spawn() {
    do_disable=1;
    if (sc_start_of_simulation_invoked()) {
     SC_REPORT_ERROR("CONNECTIONS-317", "Cannot call disable_spawn() after simulation is started");
     sc_stop();
    }
  }

  void end_of_elaboration() {
   port_t* p=this; (*p)->set_out_port_names(this->name(), this->basename());

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

  template <typename M>
  void operator()(Out<M> &rhs) {
   vld(rhs.vld);
   rdy(rhs.rdy);
   dat(rhs.dat);
#ifndef __SYNTHESIS__
   port_t* p = this;
   (*p)(rhs);
#endif
  }
 
  sc_out<bool> vld{CONN_SYNTH_NAME(this->basename(), "vld")};
  sc_in<bool> rdy{CONN_SYNTH_NAME(this->basename(), "rdy")};
  sc_out<Message> dat{CONN_SYNTH_NAME(this->basename(), "dat")};
};

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



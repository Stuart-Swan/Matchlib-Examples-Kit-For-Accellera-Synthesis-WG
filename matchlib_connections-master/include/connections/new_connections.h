
/*
new_connections.h
Stuart Swan, Platform Architect, Siemens EDA
17 Oct 2025

This is a complete rewrite of the old connections.h file.
Features & Goals:
 - "Drop in" replacement for old connections.h file.
 - Major cleanup 
 - Major simplification
 - Removal of obsolete features
 - pre-HLS sim performance improvement for DIRECT_PORT (roughly 20% improvement)
 - Separate out rand stall setup and control from base Connections implementation
 - Random stability (ie determinism) for rand stall for all sim environments, including SC+HDL sims
 - Cleaned up SC object pathnames for tracing, error reporting and back-annotation

Removed Features from old connections.h:
 - MARSHALL_PORT, SYN_PORT - use DIRECT_PORT instead
 - rdy/val/msg signal naming - use rdy/vld/dat instead
 - overloaded Connections Port binding operators to allow mismatched port types to be bound.

Changed Features from old connections.h
 - Object pathnames are cleaned up, so differ from old connections.h
 - Back-annotation API is slightly different
 - Back-annotation must be enabled via -DCONN_BACK_ANNOTATE compile flag

*/



#pragma once

#ifndef __SYNTHESIS__
#define CONNECTIONS_SIM_ONLY
#endif

#ifdef CONNECTIONS_FAST_SIM
#define AUTO_PORT Connections::TLM_PORT
#else
#define AUTO_PORT Connections::DIRECT_PORT
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
#include <memory>
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



#ifdef __SYNTHESIS__
#define CONN_SYNTH_NAME(prefix, nm) ""
#else
#define CONN_SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif


namespace Connections {

enum connections_port_t {SYN_PORT = 0, MARSHALL_PORT = 1, DIRECT_PORT = 2, TLM_PORT=3};

  class SimConnectionsClk;
  SimConnectionsClk &get_sim_clk();
  void set_sim_clk(sc_clock *clk_ptr);

  class ConManager;
  ConManager &get_conManager();

  struct force_disable_if  {
   virtual void force_disable() = 0;
  };

#ifdef CONNECTIONS_SIM_ONLY

  class SimConnectionsClk
  {
  public:
    SimConnectionsClk() { }

    void set(sc_clock *clk_ptr_) {
#ifndef HAS_SC_RESET_API
      clk_info_vector.push_back(clk_ptr_);
#endif
    }

    void pre_delay(int c) const {
      wait(adjust_for_edge(get_period_delay(c)-epsilon, c).to_seconds(),SC_SEC);
    }

    void post_delay(int c) const {
      wait(adjust_for_edge(epsilon, c).to_seconds(),SC_SEC);
    }

    inline void post2pre_delay(int c) const {
      wait(clk_info_vector[c].post2pre_delay);
    }

    inline void pre2post_delay() const {
      static sc_time delay(((2*epsilon).to_seconds()), SC_SEC);
      //caching it to optimize wall runtime
      wait(delay);
    }

    inline void period_delay(int c) const {
      wait(clk_info_vector[c].period_delay);
    }

    struct clk_info {
      clk_info(sc_clock *cp) {
        clk_ptr = cp;
        do_sync_reset = 0;
        do_async_reset = 0;
      }
      sc_clock *clk_ptr;
      sc_time post2pre_delay;
      sc_time period_delay;
      sc_time clock_edge; // time of next active edge during simulation, per clock
      bool do_sync_reset;
      bool do_async_reset;
    };

    std::vector<clk_info> clk_info_vector;

    struct clock_alias_info {
      const sc_event *sc_clock_event;
      const sc_event *alias_event;
    };
    std::vector<clock_alias_info> clock_alias_vector;

    void add_clock_alias(const sc_event &sc_clock_event, const sc_event &alias_event) {
      clock_alias_info cai;
      cai.sc_clock_event = &sc_clock_event;
      cai.alias_event = &alias_event;
      clock_alias_vector.push_back(cai);
    }

    void find_clocks(sc_object *obj) {
      sc_clock *clk = dynamic_cast<sc_clock *>(obj);
      if (clk) {
        clk_info_vector.push_back(clk);
        std::cout << "Connections Clock: " << clk->name() << " Period: " << clk->period() << std::endl;
      }

      std::vector<sc_object *> children = obj->get_child_objects();
      for ( unsigned i = 0; i < children.size(); i++ ) {
        if ( children[i] ) { find_clocks(children[i]); }
      }
    }

    void find_ccs_rtl(sc_object *obj) {
      // WAR for scverify RTL wrapper since it does not call disable_spawn for In/Out ports
      sc_module *mod = dynamic_cast<sc_module *>(obj);
      if (mod) {
        if (std::string(mod->basename()) == "ccs_rtl") {
          // std::cout << "FOUND ccs_rtl: " << mod->name() << std::endl;
          sc_object* obj = mod->get_parent_object();
          if (obj) {
            std::vector<sc_object *> children = obj->get_child_objects();
            for ( unsigned i = 0; i < children.size(); i++ ) {
              if ( children[i] ) {
                force_disable_if* f = dynamic_cast<force_disable_if*>(children[i]);
                if (f) {
                  f->force_disable();
                  // std::cout << "disabling: " << children[i]->name() << std::endl;
                }
              }
            }
          }
        }
      }

      std::vector<sc_object *> children = obj->get_child_objects();
      for ( unsigned i = 0; i < children.size(); i++ ) {
        if ( children[i] ) { find_ccs_rtl(children[i]); }
      }
    }

    void end_of_elaboration() {
      static bool done_ccs_rtl{0};
      if (done_ccs_rtl) 
        return;

      const std::vector<sc_object *> tops = sc_get_top_level_objects();

      for (unsigned i=0; i < tops.size(); i++) {
        if (tops[i]) { find_ccs_rtl(tops[i]); }
      }

      done_ccs_rtl = 1;
    }

    void start_of_simulation() {
      // Set epsilon to default value at start of simulation, after time resolution has been set
      epsilon = sc_time(0.01, SC_NS);
      const std::vector<sc_object *> tops = sc_get_top_level_objects();
      for (unsigned i=0; i < tops.size(); i++) {
        if (tops[i]) { find_clocks(tops[i]); }
      }

      for (unsigned c=0; c < clk_info_vector.size(); c++) {
        clk_info_vector[c].post2pre_delay = (sc_time((get_period_delay(c)-2*epsilon).to_seconds(), SC_SEC));
        clk_info_vector[c].period_delay = (sc_time((get_period_delay(c).to_seconds()), SC_SEC));
        clk_info_vector[c].clock_edge = adjust_for_edge(SC_ZERO_TIME,c);
      }
    }

    inline void check_on_clock_edge(int c) {
      if (clk_info_vector[c].clock_edge != sc_time_stamp()) {
        sc_process_handle h = sc_get_current_process_handle();
        std::ostringstream ss;
        ss << "Push or Pop called outside of active clock edge. \n";
        ss << "Process: " << h.name() << "\n";
        ss << "Current simulation time: " << sc_time_stamp() << "\n";
        ss << "Active clock edge: " << clk_info_vector[c].clock_edge << "\n";
        SC_REPORT_ERROR("CONNECTIONS-113", ss.str().c_str());
      }
    }

  private:

    sc_core::sc_time epsilon;

    inline sc_time get_period_delay(int c) const {
      return clk_info_vector.at(c).clk_ptr->period();
    }

    double get_duty_ratio(int c) const {
      return clk_info_vector.at(c).clk_ptr->duty_cycle();
    }

    sc_time adjust_for_edge(sc_time t, int c) const {
      if (!clk_info_vector.at(c).clk_ptr->posedge_first()) {
        return t + get_duty_ratio(c)*get_period_delay(c);
      }

      return t;
    }
  };

#define DBG_CONNECT(x)

  class Blocking_abs
  {
  public:
    Blocking_abs() {
       DBG_CONNECT("Blocking_abs CTOR: " << std::hex << (void*)this );
    };
    virtual void Post() {};
    virtual void Pre()  {};
    virtual void PrePostReset()  {};
    bool clock_registered{0};
    bool was_reset{0};
    bool disabled{0};
    int  clock_number{0};
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

    std::map<const Blocking_abs *, const sc_event *> map_port_to_event;
    std::map<const sc_event *, int> map_event_to_clock; // value is clock # + 1, so that 0 is error

    struct process_reset_info {
      process_reset_info() {
        process_ptr = 0;
        async_reset_level = false;
        sync_reset_level = false;
        async_reset_sig_if = 0;
        sync_reset_sig_if = 0;
        clk = 0;
      }
      unsigned clk;
      sc_process_b *process_ptr;
      bool        async_reset_level;
      bool        sync_reset_level;
      const sc_signal_in_if<bool> *async_reset_sig_if;
      const sc_signal_in_if<bool> *sync_reset_sig_if;
      bool operator == (const process_reset_info &rhs) {
        return (async_reset_sig_if == rhs.async_reset_sig_if) &&
               ( sync_reset_sig_if == rhs.sync_reset_sig_if) &&
               (async_reset_level  == rhs.async_reset_level) &&
               ( sync_reset_level  == rhs.sync_reset_level);
      }

      std::string to_string() {
        std::ostringstream ss;
        ss << "Process name: " << process_ptr->name();
        if (async_reset_sig_if) {
          const char *nm = "unknown";
          const sc_object *ob = dynamic_cast<const sc_object *>(async_reset_sig_if);
          sc_assert(ob);
          nm = ob->name();
          ss << " async reset signal: " << std::string(nm) << " level: " << async_reset_level ;
        }

        if (sync_reset_sig_if) {
          const char *nm = "unknown";
          const sc_object *ob = dynamic_cast<const sc_object *>(sync_reset_sig_if);
          sc_assert(ob);
          nm = ob->name();
          ss << " sync reset signal: " << std::string(nm) << " level: " << sync_reset_level ;
        }

        return ss.str();
      }
    };
    std::map<int, process_reset_info> map_clk_to_reset_info;
    std::map<sc_process_b *, process_reset_info> map_process_to_reset_info;
    bool sim_clk_initialized;

    std::vector<std::vector<Blocking_abs *>> tracked_per_clk;

    void init_sim_clk() {
      if (sim_clk_initialized) { return; }

      sim_clk_initialized = true;

      get_sim_clk().start_of_simulation();

      for (unsigned c=0; c < get_sim_clk().clk_info_vector.size(); c++) {
        map_event_to_clock[&(get_sim_clk().clk_info_vector[c].clk_ptr->posedge_event())] = c + 1; // add +1 encoding
        std::ostringstream ss, ssync;
        ss << "connections_manager_run_" << c;
        sc_spawn(sc_bind(&ConManager::run, this, c), ss.str().c_str());
        tracked_per_clk.push_back(std::vector<Blocking_abs *>());
        ssync << ss.str();
        ss << "async_reset_thread";
        sc_spawn(sc_bind(&ConManager::async_reset_thread, this, c), ss.str().c_str());
        ssync << "sync_reset_thread";
        sc_spawn(sc_bind(&ConManager::sync_reset_thread, this, c), ssync.str().c_str());
      }

      for (unsigned c=0; c < get_sim_clk().clock_alias_vector.size(); c++) {
        int resolved = map_event_to_clock[get_sim_clk().clock_alias_vector[c].sc_clock_event];
        if (resolved) {
          map_event_to_clock[get_sim_clk().clock_alias_vector[c].alias_event] = resolved;
        } else {
          SC_REPORT_ERROR("CONNECTIONS-225", "Could not resolve alias clock!");
        }
      }

      sc_spawn(sc_bind(&ConManager::check_registration, this, true), "check_registration");
    }

    void async_reset_thread(int c) {
      wait(10, SC_PS);
      process_reset_info &pri = map_clk_to_reset_info[c];

      if (pri.async_reset_sig_if == 0) { return; }

      while (1) {
        wait(pri.async_reset_sig_if->value_changed_event());
        // std::cout << "see async reset val now: " << pri.async_reset_sig_if->read() << "\n";
        get_sim_clk().clk_info_vector[c].do_async_reset =
          (pri.async_reset_sig_if->read() == pri.async_reset_level);
      }
    }

    void sync_reset_thread(int c) {
      wait(10, SC_PS);
      process_reset_info &pri = map_clk_to_reset_info[c];

      if (pri.sync_reset_sig_if == 0) { return; }

      while (1) {
        wait(pri.sync_reset_sig_if->value_changed_event());
        // std::cout << "see sync reset val now: " << pri.sync_reset_sig_if->read() << "\n";
        get_sim_clk().clk_info_vector[c].do_sync_reset =
          (pri.sync_reset_sig_if->read() == pri.sync_reset_level);
      }
    }


    void check_registration(bool b) {
      wait(50, SC_PS); // allow all Reset calls to complete, so that add_clock_event calls are all done

      bool error{0};

      // first produce list of all warnings
      for (unsigned i=0; i < tracked.size(); i++) { 
       if (tracked[i]->disabled)
         continue;

       error |= !tracked[i]->was_reset;
       if (!tracked[i]->was_reset) {
         std::string name = tracked[i]->full_name();
         SC_REPORT_WARNING("CONNECTIONS-101", ("Port or channel <" + name +
            "> wasn't reset! In thread or process '" 
            + std::string(sc_core::sc_get_current_process_b()->basename()) + "'.").c_str());
       }
      }

      if (error) {
         SC_REPORT_ERROR("CONNECTIONS-125",
           std::string("Unable to resolve clock on port - check and fix any prior warnings about missing Reset() on ports: ").c_str());
         sc_stop();
      }


      for (unsigned i=0; i < tracked.size(); i++) {
        if (tracked[i]->disabled)
          continue;

        if (!!tracked[i] && (!tracked[i]->clock_registered || (map_port_to_event[tracked[i]] == 0))) {
          DBG_CONNECT("check_registration: unreg port " << std::hex << tracked[i] << " (" << tracked[i]->full_name() << ")");

          if (!tracked[i]->clock_registered && (get_sim_clk().clk_info_vector.size() > 1)) {
            SC_REPORT_ERROR("CONNECTIONS-125",
                          (std::string("Unable to resolve clock on port - check and fix any prior warnings about missing Reset() on ports: "
                                       + tracked[i]->full_name() + " (" + tracked[i]->base_name() + ")").c_str()));
          }
        }
      }

      for (unsigned i=0; i < get_sim_clk().clk_info_vector.size(); i++) {
        std::vector<process_reset_info> v;
        decltype(map_process_to_reset_info)::iterator it;
        for (it = map_process_to_reset_info.begin(); it != map_process_to_reset_info.end(); it++) {
          if (it->second.clk == i) { v.push_back(it->second); }
        }

        if (v.size() > 1)
          for (unsigned u=0; u<v.size(); u++)
            if (!(v[0] == v[u])) {
              std::ostringstream ss;
              ss << "Two processes using same clock have different reset specs: \n"
                 << v[0].to_string() << "\n"
                 << v[u].to_string() << "\n";
              SC_REPORT_WARNING("CONNECTIONS-212", ss.str().c_str());
            }
      }
    }

    void add_clock_event(Blocking_abs *c) {
      init_sim_clk();

      if (c->clock_registered) { return; }

      c->clock_registered = true;

      class my_process : public sc_process_b
      {
      public:
#ifdef HAS_SC_RESET_API
        // Code written in restricted style to work on OSCI/Accellera sim as well as Questa and VCS
        // when SYSTEMC_HOME/sysc/kernel/sc_reset.h is available
        int static_events_size() { return m_static_events.size(); }
        const sc_event *first_event() { return m_static_events[0]; }
        std::vector<sc_reset *> &get_sc_reset_vector() { return m_resets; }
#else
        // Remove dependency on sc_reset.h, but requires user call Connections::set_sim_clk(&clk) before sc_start() 
        int static_events_size() { return 1; }
        const sc_event *first_event() { 
          SimConnectionsClk::clk_info &ci = get_sim_clk().clk_info_vector[0];
          return &(ci.clk_ptr->posedge_event());
        }
#endif
      };

      sc_process_handle h = sc_get_current_process_handle();
      sc_object *o = h.get_process_object();
      sc_process_b *b = dynamic_cast<sc_process_b *>(o);
      sc_assert(b);
      my_process *m = static_cast<my_process *>(b);
      if (m->static_events_size() != 1)
        SC_REPORT_ERROR("CONNECTIONS-112",
                        (std::string("Process does not have static sensitivity to exactly 1 event: " +
                                     std::string(h.name())).c_str()));

      const sc_event *e = m->first_event();
      map_port_to_event[c] = e;

      int clk = map_event_to_clock[e];
      if (clk <= 0) {
        SC_REPORT_ERROR("CONNECTIONS-111",
                        (std::string("Failed to find sc_clock for process: " + std::string(h.name())).c_str()));
        SC_REPORT_ERROR("CONNECTIONS-111", "Stopping sim due to fatal error.");
        sc_stop();
        return;
      }

      --clk; // undo +1 encoding for errors

      tracked_per_clk[clk].push_back(c);
      c->clock_number = clk;
      DBG_CONNECT("add_clock_event: port " << std::hex << c << " clock_number " << clk << " process " << h.name());

      sc_clock* clk_ptr = get_sim_clk().clk_info_vector[clk].clk_ptr;
      if (!clk_ptr->posedge_first()) {
                std::ostringstream ss;
                ss << "clk posedge_first() != true : process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-303", ss.str().c_str());
      }

      if (clk_ptr->start_time().value() % clk_ptr->period().value()) {
                std::ostringstream ss;
                ss << "clk start_time is not a multiple of clk period: process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-304", ss.str().c_str());
      }

#ifdef HAS_SC_RESET_API
      class my_reset : public sc_reset
      {
      public:
        const sc_signal_in_if<bool> *get_m_iface_p() { return m_iface_p; }
        std::vector<sc_reset_target> &get_m_targets() { return m_targets; }
      };

      // Here we enforce that all processes using a clock have:
      //  - consistent async resets (at most 1)
      //  - consistent sync resets  (at most 1)
      // Note that this applies to entire SC model (ie both DUT and TB)

      for (unsigned r=0; r < m->get_sc_reset_vector().size(); r++) {
        my_reset *mr = static_cast<my_reset *>(m->get_sc_reset_vector()[r]);
        sc_assert(mr);
        const sc_object *ob = dynamic_cast<const sc_object *>(mr->get_m_iface_p());
        sc_assert(ob);

        for (unsigned t=0; t < mr->get_m_targets().size(); t++)
          if (mr->get_m_targets()[t].m_process_p == b) {
            bool level = mr->get_m_targets()[t].m_level;

            process_reset_info &pri = map_process_to_reset_info[b];
            pri.process_ptr = b;
            pri.clk = clk;

            if (mr->get_m_targets()[t].m_async) {
              if (pri.async_reset_sig_if == 0) {
                pri.async_reset_sig_if = mr->get_m_iface_p();
                pri.async_reset_level = level;
              }

              if (pri.async_reset_sig_if != mr->get_m_iface_p()) {
                std::ostringstream ss;
                ss << "Mismatching async reset signal objects for same process: process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-212", ss.str().c_str());
              }

              if (pri.async_reset_level != level) {
                std::ostringstream ss;
                ss << "Mismatching async reset signal level for same process: process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-212", ss.str().c_str());
              }
            } else {
              if (pri.sync_reset_sig_if == 0) {
                pri.sync_reset_sig_if = mr->get_m_iface_p();
                pri.sync_reset_level = level;
              }

              if (pri.sync_reset_sig_if != mr->get_m_iface_p()) {
                std::ostringstream ss;
                ss << "Mismatching sync reset signal objects for same process: process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-212", ss.str().c_str());
              }

              if (pri.sync_reset_level != level) {
                std::ostringstream ss;
                ss << "Mismatching sync reset signal level for same process: process: "
                   << h.name() << " "
                   << "\n";
                SC_REPORT_ERROR("CONNECTIONS-212", ss.str().c_str());
              }
            }

            map_clk_to_reset_info[clk] = pri;
          }
      }
#endif //HAS_SC_RESET_API
    }


    void run(int clk) {
      get_sim_clk().post_delay(clk);  // align to occur just after the cycle

      SimConnectionsClk::clk_info &ci = get_sim_clk().clk_info_vector[clk];

      while (1) {
        // Post();
        for (std::vector<Blocking_abs *>::iterator it=tracked_per_clk[clk].begin(); it!=tracked_per_clk[clk].end(); ) {
          (*it)->Post();
          ++it;
        }

        get_sim_clk().post2pre_delay(clk);

        //Pre();
        for (std::vector<Blocking_abs *>::iterator it=tracked_per_clk[clk].begin(); it!=tracked_per_clk[clk].end(); ) {
          (*it)->Pre();
          ++it;
        }
        ci.clock_edge += ci.period_delay;

        if (ci.do_sync_reset || ci.do_async_reset) {
          for (std::vector<Blocking_abs *>::iterator it=tracked_per_clk[clk].begin();
               it!=tracked_per_clk[clk].end(); ++it) {
            (*it)->PrePostReset();
          }
        }

        get_sim_clk().pre2post_delay();

        if (ci.do_sync_reset || ci.do_async_reset) {
          for (std::vector<Blocking_abs *>::iterator it=tracked_per_clk[clk].begin();
               it!=tracked_per_clk[clk].end(); ++it) {
            (*it)->PrePostReset();
          }
        }
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
#ifndef HAS_SC_RESET_API
    ConManager_statics<void>::sim_clk.clk_info_vector.push_back(clk_ptr);
#endif
  }

  template <class Dummy>
  SimConnectionsClk ConManager_statics<Dummy>::sim_clk;
  template <class Dummy>
  ConManager ConManager_statics<Dummy>::conManager;

  inline SimConnectionsClk &get_sim_clk()
  {
#ifndef HAS_SC_RESET_API
    CONNECTIONS_ASSERT_MSG(ConManager_statics<void>::sim_clk.clk_info_vector.size() > 0, "You must call Connections::set_sim_clk(&clk) before sc_start()");
#endif
    return ConManager_statics<void>::sim_clk;
  }

  inline ConManager &get_conManager()
  {
    return ConManager_statics<void>::conManager;
  }

#endif // CONNECTIONS_SIM_ONLY

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

class rand_force_if {
 public:
   virtual bool force_zero() = 0;
};

class set_rand_force_if {
public:
  virtual void set_rand_force(std::shared_ptr<rand_force_if> r_if) = 0;
};

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

private:
  bool force_zero{0};
  std::shared_ptr<rand_force_if> local_rand;
  bool local_rand_set{0};

public:
 
  void set_local_rand(std::shared_ptr<rand_force_if> rif) {
    local_rand = rif;
    local_rand_set = 1;
  }

  void disable() { disabled = 1; }

  virtual void Post() {
   if (local_rand_set) {
     force_zero = local_rand->force_zero();
   }

   if (!disabled) {
     if (!in_buf_vld && !force_zero)
      rdy = 1;
     else
      rdy = 0;
   }
  }

  virtual void Pre() {
   if (!disabled && !in_buf_vld && vld && !force_zero) {
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

struct sc_trace_marker_v2 : public sc_trace_marker {
  virtual bool set_log_v2(std::ofstream *os, int &log_num, std::string &path_name) = 0;
};

template <typename Message>
struct logger : public sc_object, public sc_trace_marker_v2 {
  logger(const char* nm) : sc_object(sc_gen_unique_name(nm)) {}
 
  virtual void set_trace(sc_trace_file *trace_file_ptr) {}

  std::ofstream* log_stream;
  int log_number;
  std::string full_name{"unnamed"}; // set by start_of_sim to full_name of actual In/Out ports

  virtual bool set_log(std::ofstream *os, int &log_num, std::string &_path_name) { 
    // end_of_elab has not occurred yet, so we have to set _path_name to our sc_object name
    log_stream = os;
    _path_name = this->name();
    ++log_num;
    log_number = log_num;
    return 1; 
  }

  virtual bool set_log_v2(std::ofstream *os, int &log_num, std::string &_path_name) { 
    // caller must only call after start_of_sim, allows to use full_name to actual In/Out path
    log_stream = os;
    _path_name = full_name;
    ++log_num;
    log_number = log_num;
    return 1; 
  }

  void emit(const Message& m) {
    if (log_stream) { 
      *log_stream << std::dec << log_number << " | " << std::hex <<  m << " | " << sc_time_stamp() << "\n"; 
    }
  }
};

#endif

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Combinational ;

template <typename Message>
class Combinational_base
  : public chan_base 
  , public In_if<Message>
  , public Out_if<Message> 
#ifndef __SYNTHESIS__
  , public sc_trace_marker
  , public set_rand_force_if
#endif
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

  Combinational_base() : chan_base(sc_module_name(sc_gen_unique_name("Comb"))) {
    // This ctor needed for legacy code to compile, but cannot explicitly name vld/dat/rdy
    // else will cause naming warnings at pre-hls sim elaboration time.
    Init();
  } 

  virtual void set_trace(sc_trace_file *trace_file_ptr) {
    sc_trace(trace_file_ptr, vld, vld.name());
    sc_trace(trace_file_ptr, rdy, rdy.name());
    sc_trace(trace_file_ptr, dat, dat.name());
  }

  virtual bool set_log(std::ofstream *os, int &log_num, std::string &path_name) { return 0; }

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

  logger<Message> in_logger{"In"};
  logger<Message> out_logger{"Out"};

  void ResetWrite() {
   get_conManager().add_clock_event(&out_port);
   out_port.was_reset=1;
   out_buf_vld = 0;
   out_buf_dat = Message();
  }

  void ResetRead() {
   get_conManager().add_clock_event(&in_port);
   in_port.was_reset=1;
   in_buf_vld = 0;
  }

  Message Pop() {
   get_sim_clk().check_on_clock_edge(in_port.clock_number);

   if (!in_buf_vld) {
     do {
      wait();
     } while (!in_buf_vld);
   }

   in_logger.emit(in_buf_dat);

   in_buf_vld = 0;
   return in_buf_dat;
  }

  bool PopNB(Message& m) { 
   get_sim_clk().check_on_clock_edge(in_port.clock_number);

   if (in_buf_vld) {
    m = in_buf_dat;
    in_buf_vld = 0;
    in_logger.emit(in_buf_dat);
    return 1;
   }

   return 0;
  }

  bool PeekNB(Message& m) { 
   get_sim_clk().check_on_clock_edge(in_port.clock_number);

   if (in_buf_vld) {
    m = in_buf_dat;
    return 1;
   }

   return 0;
  }

  void Push(const Message& m) {
   get_sim_clk().check_on_clock_edge(out_port.clock_number);

   if (out_buf_vld) {
     do {
      wait();
     } while (out_buf_vld);
   }

   out_buf_vld = 1;
   out_buf_dat = m;
   out_logger.emit(out_buf_dat);
  }

  bool PushNB(const Message& m) {
   get_sim_clk().check_on_clock_edge(out_port.clock_number);

   if (!out_buf_vld) {
     out_buf_vld = 1;
     out_buf_dat = m;
     out_logger.emit(out_buf_dat);
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
    in_logger.full_name = full_name;
    portless_channel_access_in = 0;
  }
  
  void set_out_port_names(std::string full_name, std::string base_name) {
    out_port._full_name = full_name;
    out_port._base_name = base_name;
    out_logger.full_name = full_name;
    portless_channel_access_out = 0;
  }

  void start_of_simulation() {
    // at end_of_elaboration, if this channel has real user ports, then set_in/out_port_names was called.
    // At start_of_sim, if we see that set_in/out_port_names was not called,
    // then we have "port-less channel access"
    if (portless_channel_access_in) {
      in_port._full_name = std::string(name()) + ".In";
      in_port._base_name = ".In";
      in_logger.full_name = in_port._full_name;
    }
    if (portless_channel_access_out) {
      out_port._full_name = std::string(name()) + ".Out";
      out_port._base_name = ".Out";
      out_logger.full_name = out_port._full_name;
    }

    if (in_port.disabled && in_logger.log_stream) {
      sc_spawn_options opt;
      sc_spawn(sc_bind(&Combinational_base<Message>::log_thread, this), 0, &opt);
    }
  }

  void log_thread() {
    // since port is disabled, clock won't be registered. 
    // Don't even know what process port is associated with.
    // So, only thing we can do is logging only if there is a single global clock
    if (get_sim_clk().clk_info_vector.size() > 1) {
      std::cout << "Port <" << in_port.full_name() << "> has disable_spawn().\n";
      std::cout << 
        "Logging was enabled but cannot be done on this port since there are multiple system clocks.\n";
      return;
    }

    assert(get_sim_clk().clk_info_vector.size() == 1);
    sc_clock* clk_ptr = get_sim_clk().clk_info_vector[0].clk_ptr;

    while (1) {
      wait(clk_ptr->posedge_event());
      if (vld && rdy)
        in_logger.emit(dat.read());
    }
  }

  void Init() {
  }

  virtual void set_rand_force(std::shared_ptr<rand_force_if> r_if) {
    in_port.set_local_rand(r_if);
  }

#endif

  sc_signal<bool> vld;
  sc_signal<Message> dat;
  sc_signal<bool> rdy;
};

#if !defined(CONN_BACK_ANNOTATE) || defined(__SYNTHESIS__)

template <typename Message>
class Combinational<Message, DIRECT_PORT>
: public Combinational_base<Message> 
{
public:
  Combinational(sc_module_name nm) : Combinational_base<Message>(nm) {}

  Combinational() : Combinational_base<Message>() {}
};

#else

////////// BACK ANNOTATION ////////

struct back_annotation_if {
 virtual void set_latency_capacity(int _latency, int _capacity) = 0;
 virtual void get_latency_capacity(int& _latency, int& _capacity) = 0;
 virtual void get_path_names(std::string& chan, std::string& src, std::string& dst) = 0;
};

template <typename Message>
class Combinational<Message, DIRECT_PORT>
: public Combinational_base<Message> 
, public back_annotation_if
{
public:
  SC_HAS_PROCESS(Combinational);

  typedef Combinational_base<Message> base_t;

  Combinational(sc_module_name nm) : base_t(nm) {
    Init();
  }

  Combinational() : base_t() {
    Init();
  }

  void Init() {
    SC_THREAD(run);
  }

  void start_of_simulation() {
    base_t::start_of_simulation();

    if (this->in_port.disabled && this->annotated) {
     SC_REPORT_ERROR("CONNECTIONS-302", (std::string("Port <") + this->in_port.full_name() +
"> has disable_spawn() called but this cannot be used when back-annotation is also used on the channel").c_str());
     sc_stop();
    }
  }

  struct trans_t {
    Message m;
    sc_time timestamp;
  };

  tlm::tlm_fifo<trans_t> fifo{1};

  sc_event tic_event;

  bool annotated{0};
  int latency{0};
  int capacity{0};

  void set_latency_capacity(int _latency, int _capacity) {
    if (_capacity < 1)
      _capacity = 1;

   if ((_latency > 0) || (_capacity > 1))
    annotated = 1;

   fifo.nb_bound(_capacity);

   latency = _latency;
   capacity = _capacity;
  }

  void get_latency_capacity(int& _latency, int& _capacity) {
    _latency = latency;
    _capacity = capacity;
  }

  void get_path_names(std::string& chan, std::string& src, std::string& dst) {
    chan = this->name();
    src = this->out_port._full_name;
    dst = this->in_port._full_name;
  }


  sc_time period_delay() {
    return get_sim_clk().clk_info_vector[this->in_port.clock_number].period_delay;
  }

  void run() {
    if (!annotated)
      return;

    wait(100, SC_PS); // allow all Reset calls to complete, so that add_clock_event calls are all done
    assert(this->in_port.clock_registered);
    sc_clock* clk_ptr = get_sim_clk().clk_info_vector[this->in_port.clock_number].clk_ptr;
    base_t* p = this;
    while (1) {
      wait(clk_ptr->posedge_event());
      if (fifo.used() < fifo.size()) {
        Message m;
        bool b = p->base_t::PopNB(m);
        trans_t t;
        t.m = m;
        t.timestamp = sc_time_stamp() + (latency * period_delay());
        if (b)
          fifo.put(t);
      }
      tic_event.notify_delayed();
    }
  }

  void clear_fifo() {
    while (fifo.used())
      fifo.get();
  }

  void ResetRead() {
    base_t* p = this;
    p->base_t::ResetRead();
    clear_fifo();
  }

  void ResetWrite() {
    base_t* p = this;
    p->base_t::ResetWrite();
    clear_fifo();
  }

  bool PopNB(Message& m) { 
    if (!annotated) {
      base_t* p = this;
      return p->base_t::PopNB(m);
    }

    trans_t t;
    bool b = fifo.nb_peek(t);
    if (b) {
      if (sc_time_stamp() >= t.timestamp) {
        fifo.get();
        m = t.m;
        return 1;
      }
    }
    return 0;
  }

  bool PeekNB(Message& m) { 
    if (!annotated) {
      base_t* p = this;
      return p->base_t::PeekNB(m);
    }

    trans_t t;
    bool b = fifo.nb_peek(t);
    if (b) {
      if (sc_time_stamp() >= t.timestamp) {
        m = t.m;
        return 1;
      }
    }
    return 0;
  }

  Message Pop() { 
    if (!annotated) {
      base_t* p = this;
      return p->base_t::Pop();
    }

    while (1) {
      trans_t t;
      bool b = fifo.nb_peek(t);
      if (b) {
        if (sc_time_stamp() >= t.timestamp) {
          fifo.get();
          return t.m;
        }
      }

      wait(tic_event);
    }
  }
};

#endif


template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class In ;

template <typename Message>
class In <Message, DIRECT_PORT>
  : public sc_port<In_if<Message>> 
  , public force_disable_if
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

  void force_disable() {}

#else

  Message Pop() { port_t* p=this; return (*p)->Pop(); }

  bool PopNB(Message& m) { port_t* p=this; return (*p)->PopNB(m); }

  bool PeekNB(Message& m) { port_t* p=this; return (*p)->PeekNB(m); }

  void Reset() { 
    if (non_leaf_port) {
        SC_REPORT_ERROR("CONNECTIONS-102", (std::string("Port ") + this->name() + " was reset but it is a non-leaf port.").c_str());
    } else {
      port_t* p=this; (*p)->ResetRead(); 
    }
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

   get_sim_clk().end_of_elaboration();
  }

  void force_disable() {
    port_t* p=this; 
    (*p)->disable_spawn_in(); 
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
   rhs.non_leaf_port = 1;
  }
 
  bool non_leaf_port{0};

  sc_in<bool> vld{CONN_SYNTH_NAME(this->basename(), "vld")};
  sc_out<bool> rdy{CONN_SYNTH_NAME(this->basename(), "rdy")};
  sc_in<Message> dat{CONN_SYNTH_NAME(this->basename(), "dat")};
};

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Out ;

template <typename Message>
class Out <Message, DIRECT_PORT>
  : public sc_port<Out_if<Message>> 
  , public force_disable_if
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

  void force_disable() {}

#else

  void Push(const Message& m) { port_t* p = this; (*p)->Push(m); }

  bool PushNB(const Message& m) { port_t* p=this; return (*p)->PushNB(m); }

  void Reset() { 
    if (non_leaf_port) {
        SC_REPORT_ERROR("CONNECTIONS-102", (std::string("Port ") + this->name() +
              " was reset but it is a non-leaf port. In thread or process '" 
              + std::string(sc_core::sc_get_current_process_b()->basename()) + "'.").c_str());
    } else {
      port_t* p=this; (*p)->ResetWrite(); 
    }
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

   get_sim_clk().end_of_elaboration();
  }

  void force_disable() {
    port_t* p=this; 
    (*p)->disable_spawn_out(); 
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
   rhs.non_leaf_port = 1;
  }
 
  bool non_leaf_port{0};

  sc_out<bool> vld{CONN_SYNTH_NAME(this->basename(), "vld")};
  sc_in<bool> rdy{CONN_SYNTH_NAME(this->basename(), "rdy")};
  sc_out<Message> dat{CONN_SYNTH_NAME(this->basename(), "dat")};
};



///////////// TLM_PORT ///////////////////


#ifndef __SYNTHESIS__
template <typename Message>
class Combinational<Message, TLM_PORT>
  : public chan_base 
  , public In_if<Message>
  , public Out_if<Message> 
  , public sc_trace_marker
{
public:

  Combinational(sc_module_name nm) 
   : chan_base(nm) 
  {
  }

  Combinational() : chan_base(sc_module_name(sc_gen_unique_name("Comb"))) {
    // This ctor needed for legacy code to compile, but cannot explicitly name vld/dat/rdy
    // else will cause naming warnings at pre-hls sim elaboration time.
  } 

  virtual void set_trace(sc_trace_file *trace_file_ptr) { }

  virtual bool set_log(std::ofstream *os, int &log_num, std::string &path_name) { return 0; }

  tlm::tlm_fifo<Message> fifo;

  logger<Message> in_logger{"In"};
  logger<Message> out_logger{"Out"};

  void ResetWrite() {
    while (fifo.used() > 0)
      fifo.get();
  }

  void ResetRead() {
    while (fifo.used() > 0)
      fifo.get();
  }

  Message Pop() {
    Message m = fifo.get();
    in_logger.emit(m);
    return m;
  }

  bool PopNB(Message& m) { 
   bool ret = fifo.nb_get(m);
   if (ret)
     in_logger.emit(m);
   return ret;
  }

  bool PeekNB(Message& m) { 
   bool ret = fifo.nb_peek(m);
   return ret;
  }

  void Push(const Message& m) {
    fifo.put(m);
    out_logger.emit(m);
  }

  bool PushNB(const Message& m) {
    bool ret = fifo.nb_put(m);
    if (ret)
      out_logger.emit(m);

    return ret;
  }

  void disable_spawn_in() { }

  void disable_spawn_out() { }

  bool portless_channel_access_in = 1;
  bool portless_channel_access_out = 1;

  void set_in_port_names(std::string full_name, std::string base_name) {
    in_logger.full_name = full_name;
    portless_channel_access_in = 0;
  }
  
  void set_out_port_names(std::string full_name, std::string base_name) {
    out_logger.full_name = full_name;
    portless_channel_access_out = 0;
  }

  void start_of_simulation() {
    // at end_of_elaboration, if this channel has real user ports, then set_in/out_port_names was called.
    // At start_of_sim, if we see that set_in/out_port_names was not called,
    // then we have "port-less channel access"
    if (portless_channel_access_in) {
      in_logger.full_name = std::string(name()) + ".In";
    }
    if (portless_channel_access_out) {
      out_logger.full_name = std::string(name()) + ".Out";
    }
  }
};

template <typename Message>
class In <Message, TLM_PORT>
  : public sc_port<In_if<Message>> 
{
public:
  typedef sc_port<In_if<Message>> port_t;

  In(const char* nm = sc_gen_unique_name("In")) : port_t(nm) { }

  Message Pop() { port_t* p=this; return (*p)->Pop(); }

  bool PopNB(Message& m) { port_t* p=this; return (*p)->PopNB(m); }

  bool PeekNB(Message& m) { port_t* p=this; return (*p)->PeekNB(m); }

  void Reset() { port_t* p=this; (*p)->ResetRead(); }

  void disable_spawn() {}

  template <typename C>
  void operator()(C &rhs) {
   port_t* p = this;
   (*p)(rhs);
  }

  void end_of_elaboration() {
   port_t* p=this; (*p)->set_in_port_names(this->name(), this->basename());
  }
};

template <typename Message>
class Out <Message, TLM_PORT>
  : public sc_port<Out_if<Message>> 
{
public:
  typedef sc_port<Out_if<Message>> port_t;

  Out(const char* nm = sc_gen_unique_name("Out")) : port_t(nm) {}

  void Push(const Message& m) { port_t* p = this; (*p)->Push(m); }

  bool PushNB(const Message& m) { port_t* p=this; return (*p)->PushNB(m); }

  void Reset() { port_t* p=this; (*p)->ResetWrite(); }

  void disable_spawn() {}

  template <typename C>
  void operator()(C &rhs) {
   sc_port<Out_if<Message>>* p = this;
   (*p)(rhs);
  }

  void end_of_elaboration() {
   port_t* p=this; (*p)->set_out_port_names(this->name(), this->basename());
  }
};

#endif

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



// Preferred replacement for old channel_logs

struct channel_logs_v2 : public channel_logs
{
  void log_hier_helper( sc_object *obj ) {
#ifdef CONNECTIONS_SIM_ONLY
    if ( Connections::sc_trace_marker_v2 *p = dynamic_cast<Connections::sc_trace_marker_v2 *>(obj) ) {
      std::string path_name;
      if ( log_stream.is_open() && log_names.is_open() && p->set_log_v2(&log_stream, log_num, path_name) ) {
        log_names << log_num << " " << path_name << "\n";
      }
    }
    std::vector<sc_object *> children = obj->get_child_objects();
    for ( unsigned i = 0; i < children.size(); i++ ) {
      if ( children[i] ) {
        log_hier_helper(children[i]);
      }
    }
#endif
  }

  void run(sc_object* obj) {
    wait(50, SC_PS);  // Let start_of_sim occur so that In/Out path names get propagated to loggers
    log_hier_helper(obj);
  }

  void log_hierarchy( sc_object &sc_obj ) {
#ifdef CONNECTIONS_SIM_ONLY
    sc_spawn(sc_bind(&channel_logs_v2::run, this, &sc_obj), "channel_logs_v2");
#endif
  }
};

// auto-upgrade to new version:
#define channel_logs channel_logs_v2



#pragma once

#include "sc_named.h"

#ifdef __SYNTHESIS__
#include "ac_blackbox.h"
#endif

class clock_gate_module : public sc_module {
public:
   sc_in<bool> SC_NAMED(idle);
   sc_in<bool> SC_NAMED(clk_in);
   sc_out<bool> SC_NAMED(clk_out);

   sc_time period;
   sc_time period_div_2;

   // This model does work in all scenarios (pre-HLS, post-HLS sims in Questa, OSCI, etc).
   // It is not the most obvious implementation..

   // This is a SystemC simulation only model. In the RTL, this module is replaced
   // with a CGIC cell (clock gate insertion cell).
   // The key requirement of this model is to introduce no delta delays into the gated clock
   // as compared to the input clock. 

   SC_CTOR(clock_gate_module) {
#ifndef __SYNTHESIS__
     SC_THREAD(main);
#else
     ac_blackbox()
           .entity("clock_gate_module")
           .architecture("clock_gate_module")
           .library("work")
           .verilog_files("clock_gate_module.v")
           .end();
#endif
   }

#ifndef __SYNTHESIS__
   void start_of_simulation() {
     sc_clock* clk = dynamic_cast<sc_clock*>(clk_in.get_interface());
     if (clk == 0) {
       CCS_LOG("gated_clk input clock is NULL");
     }
     sc_assert(clk);
     period = clk->period();
     period_div_2 = period / 2;

     sc_signal<bool>* clk_out_sig = dynamic_cast<sc_signal<bool>*>(clk_out.get_interface());
     sc_assert(clk_out_sig);

     Connections::get_sim_clk().add_clock_alias(
      clk->posedge_event(), clk_out_sig->posedge_event());
   }

   void main() {
     while (1) {
       clk_out = 1;
       wait(period_div_2);
       // this may look wrong, but goal is to force the clk "high" when idle=1
       if (!idle.read())
         clk_out = 0;
       wait(period_div_2);
     }
   }
#endif
};


class clock_gate_module_rev1
  : public sc_signal_in_if<bool>
  , public sc_module
{
 public:
   sc_in<bool> SC_NAMED(clk_in);
   sc_in<bool> SC_NAMED(idle);

   // This model doesn't work on Questa in post-HLS sim (it does in pre-HLS sim).
   // So we cannot use it yet..

   // This is a SystemC simulation only model. In the RTL, this module is replaced
   // with a CGIC cell (clock gate insertion cell).
   // The key requirement of this model is to introduce no delta delays into the gated clock
   // as compared to the input clock. This is achieved by using immediate event notifiations rather
   // than delayed event notifications.

   SC_CTOR(clock_gate_module_rev1) {
     SC_METHOD(main);
     sensitive << clk_in;
   }

   sc_event _posedge_event;
   sc_event _negedge_event;
   sc_event _default_event;
   bool cur_val{0};

   void main() {
     // force output gated clk high when idle is true
     bool v = clk_in.read() | idle.read();
     cur_val = v;
     if (!idle.read()) {
      if (v) {
       _posedge_event.notify();
       _default_event.notify();
      } else {
       _negedge_event.notify();
      }
     }
   }

   virtual const bool& read() const { return cur_val;}
   virtual const bool& get_data_ref() const { return cur_val;}
   virtual const sc_event& value_changed_event() const { return _default_event; }
   virtual const sc_event& posedge_event() const { return _posedge_event; }
   virtual const sc_event& negedge_event() const { return _negedge_event; }
   virtual const sc_event& default_event() const { return _default_event; }

   virtual bool event() const { return 0; }
   virtual bool posedge() const { return 0; }
   virtual bool negedge() const { return 0; }
};

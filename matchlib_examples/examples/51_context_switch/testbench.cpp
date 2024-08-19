// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>

class Top : public sc_module
{
public:
  CCS_DESIGN(dut) CCS_INIT_S1(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);
  SC_SIG(context, context_in);
  SC_SIG(context, context_out);

  Connections::Combinational<uint32>  CCS_INIT_S1(out1);
  Connections::Combinational<packet>  CCS_INIT_S1(in1);
  Connections::Combinational<bool>    CCS_INIT_S1(req_context_switch);
  Connections::SyncChannel            CCS_INIT_S1(sync_channel);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);
    dut1.req_context_switch(req_context_switch);
    dut1.sync_out(sync_channel);
    dut1.context_in(context_in);
    dut1.context_out(context_out);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(sync_thread);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }


  void sync_thread() {
    sync_channel.reset_sync_in();

    context c;
    for (int j=0; j < data_size; j++) {
      c.data[j] = j;
    }

    context_in = c;
    wait();

    // this sync_thread handles the context switch synchronization with the DUT.
    // In a real system, this context state would be save/restored to reflect the
    // different "jobs" that are being mapped onto the DUT pipeline.
    // For simplicity here, we just input "random" data for the context_in port.

    for (int i = 2; 1; ++i) {
      for (int j=0; j < data_size; j++) {
        c.data[j] = j * i;
      }
      context_in = c;

      sync_channel.sync_in();
    }
  }

  void stim() {
    CCS_LOG("Stimulus started");
    in1.ResetWrite();
    req_context_switch.ResetWrite();
    wait();
    packet p;

    for (int i = 0; i < 30; i++) {
     for (int j=0; j < data_size; j++) {
      p.data[j] = i;
     }

     // every 8th input we request a context switch. 
     // In a real system the context switch request would be based on what new jobs need
     // to be mapped currently onto the DUT pipeline. And, in a real system, when a context
     // switch occurs the DUT would then be fed inputs that specifically relate to the new job.
   
     if ((i & 0x3) == 0) {
       req_context_switch.Push(1);
     }
     else {
       req_context_switch.Push(0);
     }

     in1.Push(p);
    }

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      int t = out1.Pop();
      CCS_LOG("TB resp sees: " << std::hex << t);
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  Top top("top");
  trace_hierarchy(&top, trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(top);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


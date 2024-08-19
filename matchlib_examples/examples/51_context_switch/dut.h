// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include "auto_gen_fields.h"

static const int data_size = 16;

// an input packet to the DUT pipeline
struct packet {
 uint8 data[data_size];
 AUTO_GEN_FIELD_METHODS(packet, (data) ) ;
};

// context state that is saved/restored within the DUT pipeline on each context switch
struct context {
 uint8 data[data_size];
 AUTO_GEN_FIELD_METHODS(context, (data) ) ;
};

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);

  sc_in<context> CCS_INIT_S1(context_in);  // incoming context state
  sc_out<context> CCS_INIT_S1(context_out); // outgoing context state

  Connections::In <bool> CCS_INIT_S1(req_context_switch); // request a context switch to occur if true
  Connections::In <packet> CCS_INIT_S1(in1);   // input packet
  Connections::Out<uint32> CCS_INIT_S1(out1);  // output MAC sum
  Connections::SyncOut CCS_INIT_S1(sync_out);  // sync port used during context switches

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1.Reset();
    in1.Reset();
    req_context_switch.Reset();
    sync_out.reset_sync_out();
    wait();                 

    context local_context;

    bool bypass = false;

    while (1) {
     context_out = local_context;
     // at the point where the sync_out is done we know that the pipeline is fully flushed,
     // all IO is completed,
     // and that it is safe to save/restore the context state from external source
     sync_out.sync_out();
     local_context = context_in;

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
     while (1) {
      if (!bypass) {
       if (req_context_switch.Pop() == true) {
         bypass = true;
         break;
       }
      }

      packet p = in1.Pop();
      bypass = false;
      uint32 sum = 0;

#pragma hls_unroll yes
      for (int i=0; i<data_size; i++)
        sum += p.data[i] * local_context.data[i];

      out1.Push(sum);
    }
   }
  }
};

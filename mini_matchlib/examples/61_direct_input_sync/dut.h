
#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

static const int num_direct_inputs = 8;
static const int num_samples = 3;

#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool>               SC_NAMED(clk);
  sc_in<bool>               SC_NAMED(rst_bar);

  msg_out<uint32_t>  SC_NAMED(out1);
  msg_in <uint32_t>  sample_in[num_samples];
  sync_in<>          SC_NAMED(sync_in1);

  #pragma hls_direct_input
  sc_in<uint32_t> direct_inputs[num_direct_inputs];

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    out1.reset_push();
    sync_in1.reset_pop();

    #pragma hls_unroll yes
    for (int i=0; i < num_samples; i++) {
      sample_in[i].reset_pop();
    }

    wait();  // reset state

    while (1) {
  #pragma hls_direct_input_sync all
      sync_in1.sync(); 

      #pragma hls_pipeline_init_interval 1
      #pragma hls_stall_mode flush
      for (uint32_t x=0; x < direct_inputs[0]; x++) {
        for (uint32_t y=0; y < direct_inputs[1]; y++) {
          uint32_t sum = 0;
          #pragma hls_unroll yes
          for (uint32_t s=0; s < num_samples; s++) {
            sum += sample_in[s].pop() * direct_inputs[2 + s];
          }
          uint32_t rslt = 0;
          rslt = sum * 10;
          if (rslt > direct_inputs[7]) { out1.push(rslt); }
        }
      }
    }
  }
};


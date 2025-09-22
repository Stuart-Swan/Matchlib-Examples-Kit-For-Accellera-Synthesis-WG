#include "dut.h"


#pragma hls_design top

void dut::ProcessingThread() {
  done.write(false);
  mem1_xact.Reset();
  mem2_xact.Reset();
  CCS_LOG("Processing Thread resetting");
  wait();

  CCS_LOG("Processing Thread reset");
update_loop:
  while (true) {
    do {
      wait();
    } while (!start);

#pragma hls_pipeline_init_interval 2
#pragma pipeline_stall_mode stall
    for (unsigned int idx = 0; idx < 10; ++idx) {
      mem1_xact[idx + 10] = mem1_xact[idx] + mem2_xact[idx];
      CCS_LOG("Doing vector add");
    }

    CCS_LOG("Done with vector add");
    T_data accum = 0;
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode stall
    for (unsigned int idx = 0; idx < 10; ++idx) {
      accum += mem1_xact[idx] * mem2_xact[idx];
    }

    mem1_xact[20] = accum;

    CCS_LOG("Done with dot product");
    done.write(true);
    wait();
    done.write(false);
  }
}

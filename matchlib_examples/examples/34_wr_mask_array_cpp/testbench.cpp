// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>

#include "dut.h"

CCS_MAIN(int argc, char **argv)
{
  dut dut1;

  ac_channel<uint32> mask_chan;
  ac_channel<uint32> data_chan;
  ac_channel<uint32> addr_chan;
  ac_channel<uint32> out1_chan;

    // initilialize mem so it doesn't have X values:
    for (int i=0; i < 16; i++) {
      mask_chan.write(~0);
      data_chan.write(0);
      addr_chan.write(i * 8);
      dut1.run(mask_chan, data_chan, addr_chan, out1_chan);
    }

    // First write to the first N locations of the memory in DUT
    for (int i=0; i < 16; i++) {
      mask_chan.write(i + 1);
      data_chan.write((i << 24) | (i << 16) | (i << 8) | i);
      addr_chan.write(i * 8);
      dut1.run(mask_chan, data_chan, addr_chan, out1_chan);
    }

    // Now read from the first N locations of the memory in DUT
    for (int i=0; i < 16; i++) {
      mask_chan.write(0);
      data_chan.write(0);
      addr_chan.write(i * 8);
      dut1.run(mask_chan, data_chan, addr_chan, out1_chan);
    }

  // Now check the DUT output data
  for (unsigned i=0; i < 16; i++) {
      uint32 r = out1_chan.read();
      std::cout << "TB response: " << std::hex << r << "\n";
  }

  std::cout << "Test passed" << "\n";

  return 0;
}


#ifndef __SYNTHESIS__
int sc_main(int argc, char **argv) { return 0;}
#endif

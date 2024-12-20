// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include "ac_int.h"
#include "ac_channel.h"
#include "mc_scverify.h"

#include <ac_wr_mask_array.h>


class dut 
{
public:
  dut() {}

  // last template arg==1 means we are mapping to actual byte_enable RAM
  ac_wr_mask_array_1D<uint32, 0x1000, 8, 1> mem;

#pragma hls_design interface top
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
  void CCS_BLOCK(run)(ac_channel<uint32>& mask_port, ac_channel<uint32>& data_port, ac_channel<uint32>& addr_port, ac_channel<uint32>& out1) 
  {
      uint32 mask_val = mask_port.read();
      uint32 data = data_port.read();
      uint32 addr = addr_port.read();

      if (mask_val != 0) {
        mem[addr].mask(mask_val) = data;
      } else {
        out1.write(mem[addr]);
      }
  }
};

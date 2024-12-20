// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include <ac_wr_mask_array.h>


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool>                             CCS_INIT_S1(clk);
  sc_in<bool>                             CCS_INIT_S1(rst_bar);
  Connections::Out<uint32>                CCS_INIT_S1(out1);
  Connections::In<uint32>                 CCS_INIT_S1(mask_port);
  Connections::In<uint32>                 CCS_INIT_S1(data_port);
  Connections::In<uint32>                 CCS_INIT_S1(addr_port);

  // last template arg==1 means we are mapping to actual byte_enable RAM
  ac_wr_mask_array_1D<uint32, 0x1000, 8, 1> mem;

  SC_CTOR(dut) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

private:

  void main() {
    mask_port.Reset();
    data_port.Reset();
    addr_port.Reset();
    out1.Reset();
    wait();

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      uint32 mask_val = mask_port.Pop();
      uint32 data = data_port.Pop();
      uint32 addr = addr_port.Pop();

      if (mask_val != 0) {
       mem[addr].mask(mask_val) = data;
      } else {
        out1.Push(mem[addr]);
      }
    }
  }
};

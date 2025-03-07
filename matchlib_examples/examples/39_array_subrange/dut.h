// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#include <ac_array_1D.h>
#include <ac_array_subrange.h>


#pragma hls_design top
class dut : public sc_module
{
public:
  sc_in<bool>                             SC_NAMED(clk);
  sc_in<bool>                             SC_NAMED(rst_bar);
  Connections::Out<uint32>                SC_NAMED(out1);
  Connections::In<uint32>                 SC_NAMED(mask_port);
  Connections::In<uint32>                 SC_NAMED(data_port);
  Connections::In<uint32>                 SC_NAMED(addr_port);

  ac_array_1D<uint32, 0x1000> mem;

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

      auto subrange = create_ac_array_subrange(mem, 256, 512);

      if (mask_val != 0) {
       subrange[addr] = data;
      } else {
        out1.Push(subrange[addr]);
      }
    }
  }
};

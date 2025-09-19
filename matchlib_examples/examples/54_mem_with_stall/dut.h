
#pragma once


#include <mc_connections.h>
#include "sc_named.h"
#include "auto_gen_fields.h"

#include "mem_with_stall.h"

class dut : public sc_module {
 public:
  static const unsigned int READ_ADDR1_OFFSET = 0;
  static const unsigned int READ_ADDR2_OFFSET = 8;
  static const unsigned int WRITE_ADDR_OFFSET = 16;
  static const unsigned int MEM_SIZE = 1024;
  using T_data = int;
  using T_addr = unsigned int;
  sc_in_clk SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_n);

  sc_in<bool> SC_NAMED(start);

  mem_with_stall_out<T_data, T_addr> SC_NAMED(mem1);
  mem_with_stall_out<T_data, T_addr> SC_NAMED(mem2);
  sc_out<bool> SC_NAMED(done);

  // abstract the memory interface to use array subscript
  mem_with_stall_out_xact<T_data, T_addr> SC_NAMED(mem1_xact);
  mem_with_stall_out_xact<T_data, T_addr> SC_NAMED(mem2_xact);


  SC_HAS_PROCESS(dut);

  dut(sc_module_name name) {
    SC_CTHREAD(ProcessingThread, clk.pos());
    async_reset_signal_is(rst_n, 0);
    mem1_xact.bind(clk, rst_n, mem1, mem2.stall); // swapped stall intentional
    mem2_xact.bind(clk, rst_n, mem2, mem1.stall); // swapped stall intentional
  }

  void ProcessingThread();
};

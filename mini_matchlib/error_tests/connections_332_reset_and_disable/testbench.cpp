#include <systemc.h>

#include <msg_lib.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(Disable) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::In<int> in_port{"in"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(Disable) {
    in_port(channel);
    out_port(channel);
    in_port.disable_spawn();

    SC_THREAD(dummy);
    sensitive << clk.posedge_event();
    dont_initialize();
  }

  void dummy() {
    out_port.Reset();
    in_port.Reset();
    wait();
    sc_stop();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-332", "Port <top.in> has both disable_spawn() and Reset() called");
  Disable top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


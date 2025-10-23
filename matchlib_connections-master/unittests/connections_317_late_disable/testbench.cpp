#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(LateDisable) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::In<int> in_port{"in"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(LateDisable) {
    in_port(channel);
    out_port(channel);

    SC_THREAD(controller);
    sensitive << clk.posedge_event();
    dont_initialize();
  }

  void controller() {
    in_port.Reset();
    out_port.Reset();
    wait();
    out_port.disable_spawn();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-317", "Cannot call disable_spawn() after simulation is started");
  LateDisable top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}

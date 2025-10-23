#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(AnnotatedDisable) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::In<int> in_port{"in"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(AnnotatedDisable) {
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
  unittest::expect_error("CONNECTIONS-302", "disable_spawn() called but this cannot be used when back-annotation is also used on the channel");
  AnnotatedDisable top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


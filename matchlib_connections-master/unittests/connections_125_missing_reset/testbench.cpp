#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(MissingReset) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> out_port{"out"};
  Connections::In<int> in_port{"in"};

  SC_CTOR(MissingReset) {
    out_port(channel);
    in_port(channel);

    SC_THREAD(writer);
    sensitive << clk.posedge_event();
    dont_initialize();
  }

  void writer() {
    out_port.Reset();
    wait();
    sc_stop();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-125", "Unable to resolve clock on port");
  MissingReset top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


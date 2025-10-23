#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(SkewedClock) {
  sc_clock clk{"clk", 1, SC_NS, 0.5, 0.25, SC_NS, true};
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(SkewedClock) {
    out_port(channel);

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
  unittest::expect_error("CONNECTIONS-304", "clk start_time is not a multiple of clk period");
  SkewedClock top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


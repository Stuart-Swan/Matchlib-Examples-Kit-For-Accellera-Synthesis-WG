#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(NegedgeClock) {
  sc_clock clk{"clk", 1, SC_NS, 0.5, 0, SC_NS, false};
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(NegedgeClock) {
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
  unittest::expect_error("CONNECTIONS-303", "clk posedge_first() != true");
  NegedgeClock top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(BadAlias) {
  sc_clock clk{"clk", 1, SC_NS};
  sc_event bogus_clock_event;
  sc_event alias_event;
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(BadAlias) {
    out_port(channel);
    Connections::get_sim_clk().add_clock_alias(bogus_clock_event, alias_event);

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
  unittest::expect_error("CONNECTIONS-225", "Could not resolve alias clock!");
  BadAlias top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}


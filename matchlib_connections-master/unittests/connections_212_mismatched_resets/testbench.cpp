#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(MismatchedResets) {
  sc_clock clk{"clk", 1, SC_NS};
  sc_signal<bool> rst_a{"rst_a"};
  sc_signal<bool> rst_b{"rst_b"};
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> out_port{"out"};

  SC_CTOR(MismatchedResets) {
    out_port(channel);

    SC_THREAD(writer);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_a, true);
    async_reset_signal_is(rst_b, false);
  }

  void writer() {
    rst_a = true;
    rst_b = false;
    out_port.Reset();
    wait();
    sc_stop();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-212", "Mismatching async reset signal objects for same process");
  MismatchedResets top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}

#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(BadProcess) {
  sc_clock clk{"clk", 1, SC_NS};
  sc_event trigger_event;
  Connections::Combinational<int> channel{"channel"};
  Connections::In<int> in_port{"in_port"};

  SC_CTOR(BadProcess) {
    in_port(channel);
    SC_THREAD(run);
    sensitive << trigger_event;
    dont_initialize();
  }

  void end_of_elaboration() override { trigger_event.notify(SC_ZERO_TIME); }

  void run() {
    in_port.Reset();
    sc_stop();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-111", "Failed to find sc_clock for process");
  BadProcess top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}

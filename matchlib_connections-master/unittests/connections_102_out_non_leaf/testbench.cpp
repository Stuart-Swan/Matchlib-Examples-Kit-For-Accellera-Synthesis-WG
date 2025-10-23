#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(NonLeafOut) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::Out<int> parent{"parent"};
  Connections::Out<int> child{"child"};

  SC_CTOR(NonLeafOut) {
    parent(channel);
    child(parent);

    SC_THREAD(trigger);
    sensitive << clk.posedge_event();
    dont_initialize();
  }

  void trigger() {
    channel.ResetRead();
    child.Reset();
    parent.Reset();
    sc_stop();
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-102", "Port top.parent was reset but it is a non-leaf port. In thread or process");
  NonLeafOut top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}

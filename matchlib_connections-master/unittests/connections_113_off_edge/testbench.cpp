#include <systemc.h>

#include <connections/connections.h>

#include <iostream>

#include "../common/report_utils.h"

SC_MODULE(OffEdge) {
  sc_clock clk{"clk", 1, SC_NS};
  Connections::Combinational<int> channel{"channel"};
  Connections::In<int> in_port{"in_port"};
  Connections::Out<int> out_port{"out_port"};

  SC_CTOR(OffEdge) {
    in_port(channel);
    out_port(channel);

    SC_THREAD(producer);
    sensitive << clk.posedge_event();
    dont_initialize();

    SC_THREAD(consumer);
    sensitive << clk.posedge_event();
    dont_initialize();
  }

  void producer() {
    out_port.Reset();
    wait();
    wait(0.5, SC_NS);
    Connections::get_sim_clk().check_on_clock_edge(channel.out_port.clock_number);
    sc_stop();
  }

  void consumer() {
    in_port.Reset();
    while (true) {
      wait();
    }
  }
};

int sc_main(int argc, char **argv) {
  unittest::expect_error("CONNECTIONS-113", "Push or Pop called outside of active clock edge");
  OffEdge top{"top"};

  sc_start();

  const bool pass = unittest::finalize_expectation();
  std::cout << (pass ? "Test PASSED" : "Test FAILED") << std::endl;
  return pass ? 0 : 1;
}

// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>

#pragma hls_design top
class and_gate : public sc_module
{
public:
  sc_in<bool>  SC_NAMED(in1);
  sc_in<bool>  SC_NAMED(in2);
  sc_out<bool> SC_NAMED(out1);

  SC_CTOR(and_gate) {
    SC_METHOD(run);
    sensitive << in1 << in2;
  }

  void run() {
    out1 = in1.read() & in2.read();
  }
};


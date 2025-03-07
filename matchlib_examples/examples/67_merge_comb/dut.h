// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <mc_connections.h>
#include "auto_gen_fields.h"

#include "mc_toolkit_utils.h"

typedef uint16 type1_t;
typedef uint16 type2_t;
typedef uint16 type3_t;

struct out_t {
  type1_t f1;
  type2_t f2;
  type3_t f3;

  AUTO_GEN_FIELD_METHODS(out_t, ( \
     f1 \
   , f2 \
   , f3 \
  ) )
};

#pragma hls_design top
class merge_comb : public sc_module {
public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  Connections::In <type1_t> SC_NAMED(in1);
  Connections::In <type2_t> SC_NAMED(in2);
  Connections::In <type2_t> SC_NAMED(in3);
  Connections::Out<out_t> SC_NAMED(out1);

  // Merge 3 In<> inputs into 1 Out<> output.
  // This module is entirely combinational logic (no sequential logic).

  SC_CTOR(merge_comb) {
    SC_METHOD(out1_vld_method);
    dont_initialize();
    sensitive << in1.vld << in2.vld << in3.vld;

    SC_METHOD(merge_method);
    dont_initialize();
    sensitive << in1.dat << in2.dat << in3.dat;

    SC_METHOD(in_rdy_method);
    dont_initialize();
    sensitive << out1.rdy << in1.vld << in2.vld << in3.vld;

#ifdef CONNECTIONS_SIM_ONLY
    in1.disable_spawn();
    in2.disable_spawn();
    in3.disable_spawn();
    out1.disable_spawn();
#endif
  }

  void in_rdy_method() {
    in1.rdy = out1.rdy && in2.vld && in3.vld;
    in2.rdy = out1.rdy && in3.vld && in1.vld;
    in3.rdy = out1.rdy && in1.vld && in2.vld;
  }

  void out1_vld_method() {
    out1.vld = in1.vld.read() && in2.vld.read() && in3.vld.read();
  }

  void merge_method() {
    type1_t p1;
    type1_t p2;
    type1_t p3;

    bits_to_type_if_needed(p1, in1.dat);
    bits_to_type_if_needed(p2, in2.dat);
    bits_to_type_if_needed(p3, in3.dat);

    out_t o1;

    o1.f1 = p1;
    o1.f2 = p2;
    o1.f3 = p3;

    type_to_bits_if_needed(out1.dat, o1);
  }
};


#pragma once

#if defined(__SYNTHESIS__) || defined(CCS_SYSC)
#include <ac_assert_no_x.h>
#else

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

#include <ac_assert.h>
#ifdef __SYNTHESIS__
#include <ac_blackbox.h>
#endif

#ifndef SYSTEMC_INCLUDED
#include <systemc.h>
#endif
#ifdef SYSTEMC_INCLUDED
#include <mc_typeconv.h>
#include <ccs_types.h>
#endif

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

namespace ac {
template <typename T>
  class x_assert_bb {
    public:
      x_assert_bb() {}

      #pragma hls_design interface blackbox
      void run(T din, bool &expr){
#ifdef __SYNTHESIS__
        ac_blackbox()
          .entity("x_assert_bb")
          .verilog_files("x_assert/x_assert_bb.v")
          .vhdl_files("x_assert/x_assert_bb.vhd")
          .parameter("WIDTH", mc_typedef_T_traits<T>::bitwidth)
          .outputs("expr")
          .area(0.001)
          .delay(0)
          .end();
#endif
        sc_lv<mc_typedef_T_traits<T>::bitwidth> vec;
        type_to_vector(din, mc_typedef_T_traits<T>::bitwidth, vec);
        expr = vec.xor_reduce() != sc_logic(sc_dt::SC_LOGIC_X); 
      }
  };

  // Synthesizable assert "ac_assert_any_x" to check if the expression has any bit as "x"
  template <typename T>
  inline void ac_assert_no_x(const char *filename, int lineno, const char *expr_str, T din) {
    static x_assert_bb<T> x_check_obj; 
    bool expr=1;
    x_check_obj.run(din, expr);
    ac::ac_assert(filename, lineno, expr_str, expr);
  }
}
#ifdef __AC_NAMESPACE
}
#endif

#define assert_no_x(expr) ac::ac_assert_no_x(__FILE__, __LINE__, #expr, expr)

#endif

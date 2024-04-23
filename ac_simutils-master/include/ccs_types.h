/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Simulation Utilities                               *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Sun Feb  4 15:24:00 PST 2024                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright 2020 Siemens                                                *
 *                                                                        *
 **************************************************************************
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      * 
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *      http://www.apache.org/licenses/LICENSE-2.0                        *
 *                                                                        *
 *  Unless required by applicable law or agreed to in writing, software   * 
 *  distributed under the License is distributed on an "AS IS" BASIS,     * 
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or       *
 *  implied.                                                              * 
 *  See the License for the specific language governing permissions and   * 
 *  limitations under the License.                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package is available at github.       *
 *                                                                        *
 *************************************************************************/
//*****************************************************************************************
// File: ccs_types.h
//
// Description: Provides helper functions to convert between datatypes.
//
// Revision History:
//    1.5.0 - Add mc_typedef_T_traits<sc_logic>
//    1.2.1 - Initial version on github
//*****************************************************************************************

//------------------------------------------------------------------------------
// Catapult Synthesis - Sample I/O Port Library
//
// This document may be used and distributed without restriction provided that
// this copyright statement is not removed from the file and that any derivative
// work contains this copyright notice.
//
// The design information contained in this file is intended to be an example
// of the functionality which the end user may study in preparation for creating
// their own custom interfaces. This design does not necessarily present a
// complete implementation of the named protocol or standard.
//
//------------------------------------------------------------------------------

#ifndef __CCS_TYPES_H
#define __CCS_TYPES_H

#include <systemc.h>

#include <tlm.h>

#undef for
#include <sstream>

// Includes all AC types
#include <ac_sc.h>
#include <ac_complex.h>
#include <mc_typeconv.h>

#ifndef CALYPTO_SYSC
#define ccs_concat(n1,n2) (n1 ? ((std::string(n1)+"_"+n2).c_str()) : 0)
#else
#define ccs_concat(n1,n2) (n2)
#endif

#ifndef P2P_DEFAULT_VIEW
#define P2P_DEFAULT_VIEW TLM
#endif

// This enumeration defines the abstraction of channels and ports
//  TLM - Uses an abstract, transaction based interconnect object, usually an array or TLM FIFO.
//  SYN - Uses signal based interconnect objects.  This view is always used by synthesis.
//  AUTO - Default setting.  Channel is TLM in source simulation and SYN for synthesis and SCVerify.
enum abstraction_t {TLM = 0, SYN = 1, AUTO = 2};

namespace mc_typedef_T_traits_private
{
  // helper structs for statically computing log2 like functions (nbits, log2_floor, log2_ceil)
  //   using recursive templates
  template<unsigned char N>
  struct s_N {
    template<unsigned X>
    struct s_X {
      enum {
        X2 = X >> N,
        N_div_2 = N >> 1,
        nbits = X ? (X2 ? N + s_N<N_div_2>::template s_X<X2>::nbits : s_N<N_div_2>::template s_X<X>::nbits) : 0
      };
    };
  };
  template<> struct s_N<0> {
    template<unsigned X>
    struct s_X {
      enum {nbits = !!X };
    };
  };
};

// compiler time constant for log2 like functions
template<unsigned X>
struct nbits {
  enum { val = mc_typedef_T_traits_private::s_N<16>::s_X<X>::nbits };
};


// Helper struct for determining bitwidth of types
template <class T> struct mc_typedef_T_traits;

// INT <-> SC_LV
template <>
struct mc_typedef_T_traits< int > {
  enum { bitwidth = 32,
         issigned = 1
       };
};

// UINT <-> SC_LV
template <>
struct mc_typedef_T_traits< unsigned int > {
  enum { bitwidth = 32,
         issigned = 0
       };
};

// SHORT <-> SC_LV
template <>
struct mc_typedef_T_traits< short > {
  enum { bitwidth = 16,
         issigned = 1
       };
};

// USHORT <-> SC_LV
template <>
struct mc_typedef_T_traits< unsigned short > {
  enum { bitwidth = 16,
         issigned = 0
       };
};

// CHAR <-> SC_LV
template <>
struct mc_typedef_T_traits< char > {
  enum { bitwidth = 8,
         issigned = 1
       };
};

// UCHAR <-> SC_LV
template <>
struct mc_typedef_T_traits< unsigned char > {
  enum { bitwidth = 8,
         issigned = 0
       };
};

// LONG LONG <-> SC_LV
template <>
struct mc_typedef_T_traits< long long > {
  enum { bitwidth = 64,
         issigned = 1
       };
};

// ULONG LONG <-> SC_LV
template <>
struct mc_typedef_T_traits< unsigned long long > {
  enum { bitwidth = 64,
         issigned = 0
       };
};

// LONG <-> SC_LV
template <>
struct mc_typedef_T_traits< long > {
  enum { bitwidth = 32,
         issigned = 1
       };
};

// ULONG <-> SC_LV
template <>
struct mc_typedef_T_traits< unsigned long > {
  enum { bitwidth = 32,
         issigned = 0
       };
};

// BOOL <-> SC_LV
template <>
struct mc_typedef_T_traits< bool > {
  enum { bitwidth = 1,
         issigned = 0
       };
};


// SC_LV <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_lv<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};

// SC_UINT <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_uint<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};

// SC_BV <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_bv<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};

// SC_BIT <-> SC_LV
template<>
struct mc_typedef_T_traits< sc_bit > {
  enum { bitwidth = 1,
         issigned = 0
       };
};

// SC_LOGIC <-> SC_LV
template<>
struct mc_typedef_T_traits< sc_logic > {
  enum { bitwidth = 1,
         issigned = 0
       };
};

// SC_INT <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_int<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 1
       };
};

// SC_BIGUINT <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_biguint<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};

// SC_BIGINT <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< sc_bigint<Twidth> > {
  enum { bitwidth = Twidth,
         issigned = 1
       };
};

#if defined(SC_INCLUDE_FX)
// SC_FIXED <-> SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
struct mc_typedef_T_traits< sc_fixed<Twidth,Ibits,Qmode,Omode,Nbits> > {
  enum { bitwidth = Twidth,
         issigned = 1
       };
};
// SC_UFIXED <-> SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
struct mc_typedef_T_traits< sc_ufixed<Twidth,Ibits,Qmode,Omode,Nbits> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};
#endif

// AC_INT (signed) <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< ac_int<Twidth,true> > {
  enum { bitwidth = Twidth,
         issigned = 1
       };
};

// AC_INT (unsigned) <-> SC_LV
template <int Twidth>
struct mc_typedef_T_traits< ac_int<Twidth,false> > {
  enum { bitwidth = Twidth,
         issigned = 0
       };
};

// AC_FIXED (signed) <-> SC_LV
template<int Twidth, int Ibits, bool Signed, ac_q_mode Qmode, ac_o_mode Omode>
struct mc_typedef_T_traits< ac_fixed<Twidth,Ibits,Signed,Qmode,Omode> > {
  enum { bitwidth = Twidth,
         issigned = Signed?1:0
       };
};

#ifdef __AC_FLOAT_H
// Guard added because SLEC doesn't have ac_float support yet
// AC_FLOAT (unsigned only) <-> SC_LV
template<int MTbits, int MIbits, int Ebits, ac_q_mode Qmode>
struct mc_typedef_T_traits<ac_float<MTbits,MIbits,Ebits,Qmode> > {
  enum { bitwidth = MTbits+Ebits,
         issigned = 0
       };
};
#endif

template<class T>  // Template arguments may be added as needed
struct mc_typedef_T_traits< ac_complex<T> > {
  enum { bitwidth = mc_typedef_T_traits<T>::bitwidth * 2, // Requires bitwidth trait for based type
         issigned = false
       };
};

template<class T>
inline  void type_to_vector(const ac_complex<T> &in, int length, sc_lv<mc_typedef_T_traits< ac_complex<T> >::bitwidth> &rvec)
{
  sc_lv<mc_typedef_T_traits<T>::bitwidth> vec_r, vec_i;
  type_to_vector(in.r(),mc_typedef_T_traits<T>::issigned,vec_r);
  type_to_vector(in.i(),mc_typedef_T_traits<T>::issigned,vec_i);
  rvec = (vec_r << mc_typedef_T_traits<T>::bitwidth) | vec_i;
}

template<class T>
inline  void vector_to_type(const sc_lv<mc_typedef_T_traits< ac_complex<T> >::bitwidth> &in, bool issigned, ac_complex<T> *result)
{
  sc_lv<mc_typedef_T_traits<T>::bitwidth> vec_r, vec_i;
  vec_r = in >> mc_typedef_T_traits<T>::bitwidth;
  vec_i = in;
  T r, i;
  vector_to_type(vec_r,mc_typedef_T_traits<T>::issigned,&r);
  vector_to_type(vec_i,mc_typedef_T_traits<T>::issigned,&i);
  result->set_r(r);
  result->set_i(i);
}

// Helper Classes for checking that reset was called and all ports are bound
class p2p_checker
{
  mutable bool is_ok;
#ifndef __SYNTHESIS__
  const char *objname;
  std::stringstream error_string;
#endif

public:
  p2p_checker (const char *name, const char *func_name, const char *operation) :
    is_ok(false) {
#ifndef __SYNTHESIS__
    objname = name;
    error_string << "You must " << func_name << " before you can " << operation << ".";
#endif
  }

  inline void ok () {
    is_ok = true;
  }

  inline void test () const {
#ifndef __SYNTHESIS__
    if ( !is_ok ) {
      SC_REPORT_ERROR(objname, error_string.str().c_str());
      is_ok = true; // Only report error message one time
    }
#endif
  }
};

#endif


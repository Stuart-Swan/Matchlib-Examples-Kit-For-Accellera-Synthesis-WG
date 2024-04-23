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
// File: mc_typeconv.h
//
// Description: Provides helper functions to convert between datatypes.
//
// Revision History:
//    1.2.1 - Initial version on github
//*****************************************************************************************

//-------------------------------------------------
// Conversion functions from various types to sc_lv
//-------------------------------------------------

#include "mc_container_types.h"

// Check for macro definitions that will conflict with template parameter names in this file
#if defined(Twidth)
#define Twidth 0
#error The macro name 'Twidth' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Twidth' giving the location of the previous definition.
#endif
#if defined(Ibits)
#define Ibits 0
#error The macro name 'Ibits' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Ibits' giving the location of the previous definition.
#endif
#if defined(Qmode)
#define Qmode 0
#error The macro name 'Qmode' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Qmode' giving the location of the previous definition.
#endif
#if defined(Omode)
#define Omode 0
#error The macro name 'Omode' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Omode' giving the location of the previous definition.
#endif
#if defined(Nbits)
#define Nbits 0
#error The macro name 'Nbits' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Nbits' giving the location of the previous definition.
#endif
#if defined(Tclass)
#define Tclass 0
#error The macro name 'Tclass' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tclass' giving the location of the previous definition.
#endif
#if defined(TclassW)
#define TclassW 0
#error The macro name 'TclassW' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'TclassW' giving the location of the previous definition.
#endif

#if !defined(MC_TYPECONV_H)
//#define MC_TYPECONV_H

// built-in types and sc_types:

template < int Twidth>
void type_to_vector(const sc_int<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  rvec = in;
}

template < int Twidth>
void type_to_vector(const sc_bigint<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  rvec = in;
}
#endif

//This section was further down in the file, however, gcc 4.x is complaining about "ambiguous operator overloads" that
//seem to be remedied by moving this section here (and adding the type_to_vector template functions just above. PT
#if defined(SC_FIXED_H) && !defined(MC_TYPECONV_H_SC_FIXED)
#define MC_TYPECONV_H_SC_FIXED

// ---------------------------------------------------------
// ---------------------------------  SC_FIXED
// SC_FIXED => SC_LV
template<int Twidth, int Ibits>
void type_to_vector(const sc_fixed<Twidth,Ibits> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  // rvec = in.range(Twidth-1, 0); // via assignment from sc_bv_base not provided
  sc_int<Twidth> tmp;
  tmp = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_FIXED => SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void type_to_vector(const sc_fixed<Twidth,Ibits,Qmode,Omode,Nbits> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  // rvec = in.range(Twidth-1, 0); // via assignment from sc_bv_base not provided
  sc_bigint<Twidth> tmp;
  tmp = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => SC_FIXED
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_fixed<Twidth,Ibits,Qmode,Omode,Nbits> *result)
{
  sc_bigint<Twidth> tmp = in;
  result->range(Twidth-1, 0) = tmp; // via sc_bv_base
}

// ---------------------------------------------------------
// ---------------------------------  SC_UFIXED
// SC_UFIXED => SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void type_to_vector(const sc_ufixed<Twidth,Ibits,Qmode,Omode,Nbits> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  // rvec = in.range(Twidth-1, 0); // via assignment from sc_bv_base not provided
  sc_bigint<Twidth> tmp;
  tmp = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => SC_UFIXED
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_ufixed<Twidth,Ibits,Qmode,Omode,Nbits> *result)
{
  sc_bigint<Twidth> tmp = in;
  result->range(Twidth-1, 0) = tmp;
}

#if 0
// Need to leave these out since systemc 2.2 does not define the _fast classes.  When 2.3 is used,
// then these conversions allow a better "Not Allowed in sysnthesis" message is possible.
// SC_FIXED_FAST => SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void type_to_vector(const sc_fixed_fast<Twidth,Ibits,Qmode,Omode,Nbits> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  // rvec = in.range(Twidth-1, 0); // via assignment from sc_bv_base not provided
  sc_bigint<Twidth> tmp;
  tmp = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => SC_FIXED_FAST
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_fixed_fast<Twidth,Ibits,Qmode,Omode,Nbits> *result)
{
  sc_bigint<Twidth> tmp = in;
  result->range(Twidth-1, 0) = tmp; // via sc_bv_base
}

// SC_UFIXED_FAST => SC_LV
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void type_to_vector(const sc_ufixed_fast<Twidth,Ibits,Qmode,Omode,Nbits> &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  // rvec = in.range(Twidth-1, 0); // via assignment from sc_bv_base not provided
  sc_bigint<Twidth> tmp;
  tmp = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => SC_UFIXED_FAST
template<int Twidth, int Ibits, sc_q_mode Qmode, sc_o_mode Omode, int Nbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_ufixed_fast<Twidth,Ibits,Qmode,Omode,Nbits> *result)
{
  sc_bigint<Twidth> tmp = in;
  result->range(Twidth-1, 0) = tmp;
}
#endif

// ---------------------------------------------------------
#endif

#if !defined(MC_TYPECONV_H)
#define MC_TYPECONV_H

// GENERIC => SC_LV
template <class T, int Twidth>
void type_to_vector(const T &in, int length, sc_lv<Twidth> &rvec)
{
  // sc_assert(length == Twidth);
  rvec = in;
}


// ---------------------------------------------------------
// ---------------------------------  SC_INT
// SC_INT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => SC_INT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_int<Twidth> *result)
{
  *result = in;
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// ---------------------------------  SC_UINT
// SC_UINT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => SC_UINT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_uint<Twidth> *result)
{
  *result = in;
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// ---------------------------------  SC_BIGINT
// SC_BIGINT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => SC_BIGINT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_bigint<Twidth> *result)
{
  *result = in;
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// ---------------------------------  SC_BIGUINT
// SC_BIGUINT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => SC_BIGUINT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_biguint<Twidth> *result)
{
  *result = in;
}
// ---------------------------------------------------------


// ---------------------------------------------------------
// ---------------------------------  BOOL
// BOOL => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => BOOL
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, bool *result)
{
  *result = (in[0] == sc_dt::Log_1 ? true : false);
}
// ---------------------------------------------------------


// ---------------------------------------------------------
// ---------------------------------  ***SPECIAL*** sc_logic to bool, bool to sc_logic
// SC_Logic => BOOL
inline
void vector_to_type(const sc_logic &in, bool issigned, bool *result)
{
  *result = (in == sc_dt::Log_1 ? true : false);
}

// BOOL => SC_Logic
inline
void type_to_vector(const bool &in, int length, sc_logic &rvec)
{
  rvec = in;
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// ---------------------------------  ***SPECIAL*** sc_logic to sc_uint<1>, sc_uint<1> to sc_logic
// Should not really ever need this, but for completeness...
// SC_Logic => SC_UINT<1>
inline
void vector_to_type(const sc_logic &in, bool issigned, sc_uint<1> *result)
{
  *result = (in == sc_dt::Log_1 ? 1 : 0);
}

// SC_UINT<1> => SC_Logic
inline
void type_to_vector(const sc_uint<1> &in, int length, sc_logic &rvec)
{
  rvec = in ? sc_dt::Log_1 : sc_dt::Log_0;
}

// ---------------------------------  ***SPECIAL*** sc_lv<1> to sc_logic, sc_logic to sc_lv<1>
inline
void vector_to_type(const sc_lv<1> &in, bool issigned, sc_logic *result)
{
  *result = in[0];
}

inline
void type_to_vector(const sc_logic &in, int length, sc_lv<1> &rvec)
{
  rvec[0] = in;
}
// ---------------------------------------------------------

// ---------------------------------  ***SPECIAL*** sc_logic to sc_lv<1>, sc_lv<1> to sc_logic
inline
void vector_to_type(const sc_logic &in, bool issigned, sc_lv<1> *result)
{
  (*result)[0] = in;
}

inline
void type_to_vector(const sc_lv<1> &in, int length, sc_logic &rvec)
{
  rvec = in[0];
}
// ---------------------------------------------------------

// ---------------------------------  ***SPECIAL*** sc_logic to sc_logic, sc_logic to sc_logic
inline
void vector_to_type(const sc_logic &in, bool issigned, sc_logic *result)
{
  *result = in;
}

inline
void type_to_vector(const sc_logic &in, int length, sc_logic &rvec)
{
  rvec = in;
}
// ---------------------------------------------------------

// ---------------------------------  ***SPECIAL*** sc_lv<N> to sc_lv<N>,
template<int Twidth>
inline
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_lv<Twidth> *result)
{
  *result = in;
}

template<int Twidth>
inline
void type_to_vector(const sc_lv<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  rvec = in;
}
// ---------------------------------------------------------

// ---------------------------------  sc_bv<N> to sc_lv<N>, sc_lv<N> to sc_bv<N>
template<int Twidth>
inline
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, sc_bv<Twidth> *result)
{
  *result = in;
}

template<int Twidth>
inline
void type_to_vector(const sc_bv<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  rvec = in;
}
// ---------------------------------------------------------

// ---------------------------------  ***SPECIAL*** sc_lv<1> to sc_bit, sc_bit to sc_lv<1>
inline
void vector_to_type(const sc_lv<1> &in, bool issigned, sc_bit *result)
{
  *result = in[0];
}

inline
void type_to_vector(const sc_bit &in, int length, sc_lv<1> &rvec)
{
  rvec = in;
}
// ---------------------------------------------------------

template<int Twidth, class T>
void vector_to_type_builtin(const sc_lv<Twidth> &in, bool issigned, T *result)
{
  // sc_assert(sizeof(T) <= sizeof(int));
  if (issigned) {
    *result = in.to_int();
  } else {
    *result = in.to_uint();
  }
}

template<int Twidth, class T>
void vector_to_type_builtin_64(const sc_lv<Twidth> &in, bool issigned, T *result)
{
  // sc_assert(sizeof(T) * CHAR_BIT <= 64);
  if (issigned) {
    sc_int<Twidth < 64 ? Twidth : 64> i;
    i = in;
    *result = i.to_int64();
  } else {
    sc_uint<Twidth < 64 ? Twidth : 64> u;
    u = in;
    *result = u.to_uint64();
  }
}


// ---------------------------------------------------------
// --------------------------------- GENERIC

template<int Twidth, class T>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, T *result)
{
  // sc_assert(sizeof(T) <= sizeof(int));
  //assert(Twidth<=sizeof(T)*CHAR_BIT);
  if (sizeof(T) <= sizeof(int)) {
    if (issigned) {
      *result = static_cast<T> (in.to_int());
    } else {
      *result = static_cast<T> (in.to_uint());
    }
  } else {
    if (issigned) {
      sc_int<Twidth < 64 ? Twidth : 64> i;
      i = in;
      *result = static_cast<T> (i.to_int64());
    } else {
      sc_uint<Twidth < 64 ? Twidth : 64> u;
      u = in;
      *result = static_cast<T> (u.to_uint64());
    }
  }
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// --------------------------------- CHAR
// CHAR => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => CHAR
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, char *result)
{
  vector_to_type_builtin(in, issigned, result);
}
// ---------------------------------------------------------


// ---------------------------------------------------------
// --------------------------------- UNSIGNED CHAR
// UNSIGNED CHAR => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => UNSIGNED CHAR
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, unsigned char *result)
{
  vector_to_type_builtin(in, issigned, result);
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// --------------------------------- SIGNED CHAR
// SC_LV => SIGNED CHAR
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, signed char *result)
{
  vector_to_type_builtin(in, issigned, result);
}
// ---------------------------------------------------------


// ---------------------------------------------------------
// --------------------------------- SHORT
// SHORT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => SHORT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, short *result)
{
  vector_to_type_builtin(in, issigned, result);
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// --------------------------------- UNSIGNED SHORT
// UNSIGNED SHORT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => UNSIGNED SHORT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, unsigned short *result)
{
  vector_to_type_builtin(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- INT
// INT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => INT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, int *result)
{
  vector_to_type_builtin(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- UNSIGNED INT
// UNSIGNED INT => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => UNSIGNED INT
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, unsigned int *result)
{
  vector_to_type_builtin(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- LONG
// LONG => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => LONG
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, long *result)
{
  vector_to_type_builtin_64(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- UNSIGNED LONG
// UNSIGNED LONG => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => UNSIGNED LONG
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, unsigned long *result)
{
  vector_to_type_builtin_64(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- LONG LONG
// LONG LONG => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => LONG LONG
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, long long *result)
{
  vector_to_type_builtin_64(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- UNSIGNED LONG LONG
// UNSIGNED LONG LONG => SC_LV
// (uses GENERIC type_to_vector)

// SC_LV => UNSIGNED LONG LONG
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, unsigned long long *result)
{
  vector_to_type_builtin_64(in, issigned, result);
}

// ---------------------------------------------------------
// --------------------------------- DOUBLE
template <int Twidth>
void type_to_vector(const double &in, int length, sc_lv<Twidth> &rvec)
{
  long long tmp;
  std::memcpy(&tmp, &in, sizeof(double));
  type_to_vector(tmp, length, rvec);
}

template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, double *result)
{
  long long res;
  vector_to_type(in, issigned, &res);
  std::memcpy(result, &res, sizeof(double));
}
// ---------------------------------------------------------

// ---------------------------------------------------------
// --------------------------------- FLOAT
template <int Twidth>
void type_to_vector(const float &in, int length, sc_lv<Twidth> &rvec)
{
  int tmp;
  std::memcpy(&tmp, &in, sizeof(float));
  type_to_vector(tmp, length, rvec);
}

template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, float *result)
{
  int res;
  vector_to_type(in, issigned, &res);
  std::memcpy(result, &res, sizeof(float));
}
// ---------------------------------------------------------


// ---------------------------------------------------------
// --------------------------------- CONTAINER
// CONTAINER mgc_sysc_ver_array1D => SC_LV
template <class Tclass, int V, int Twidth>
void type_to_vector(const mgc_sysc_ver_array1D<Tclass,V> &in, int length, sc_lv<Twidth> &rvec)

{
  // sc_assert(Twidth == length);
  // sc_assert(Twidth % V == 0);
  const int element_length = Twidth / V;
  sc_lv<element_length> el_vec;
  for (int i = 0; i < V; ++i) {
    type_to_vector(in[i], element_length, el_vec);
    rvec.range((i + 1) * element_length - 1, i * element_length) = el_vec;
  }
}

// SC_LV => CONTAINER mgc_sysc_ver_array1D
template <int Twidth, class Tclass, int TclassW>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, mgc_sysc_ver_array1D<Tclass,TclassW> *result)

{
  // sc_assert(Twidth > 0 && Twidth % TclassW == 0);
  enum { ew = Twidth/TclassW };
  for (int i = 0; i < TclassW; ++i) {
    sc_lv<ew> tmp = in.range((i + 1) * ew - 1, i * ew);
    vector_to_type(tmp, issigned, &result->operator[](i));
  }
}

#endif


#if defined(__AC_INT_H) && !defined(MC_TYPECONV_H_AC_INT)
#define MC_TYPECONV_H_AC_INT

#include <ac_sc.h>

// AC_INT => SC_LV
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ac_int<Twidth,true> *result)
{
  sc_bigint<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  *result = to_ac(tmp);
}

template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ac_int<Twidth,false> *result)
{
  sc_biguint<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  *result = to_ac(tmp);
}

// SC_LV => AC_INT
template<int Twidth>
void type_to_vector(const ac_int<Twidth,true> &in, int length, sc_lv<Twidth> &rvec)
{
  sc_bigint<Twidth> tmp;
  tmp = to_sc(in);
  type_to_vector(tmp, length, rvec);
}

template<int Twidth>
void type_to_vector(const ac_int<Twidth,false> &in, int length, sc_lv<Twidth> &rvec)
{
  sc_biguint<Twidth> tmp;
  tmp = to_sc(in);
  type_to_vector(tmp, length, rvec);
}

// SC_LOGIC => AC_INT
inline
void vector_to_type(const sc_logic &in, bool issigned, ac_int<1,false> *result)
{
  sc_uint<1> tmp;
  vector_to_type(in,false,&tmp);
  *result = tmp.to_uint();
}

// AC_INT => SC_LOGIC
inline
void type_to_vector(const ac_int<1,false> &in, int length, sc_logic &rvec)
{
  sc_uint<1> tmp = in.to_uint();
  type_to_vector(tmp,1,rvec);
}

#endif

#if defined(__AC_FIXED_H) && !defined(MC_TYPECONV_H_AC_FIXED)
#define MC_TYPECONV_H_AC_FIXED

#include "ac_sc.h"

// AC_FIXED => SC_LV
template<int Twidth, int Ibits, bool Signed, ac_q_mode Qmode, ac_o_mode Omode>
void type_to_vector(const ac_fixed<Twidth,Ibits,Signed,Qmode,Omode> &in, int length, sc_lv<Twidth> &rvec)
{
  ac_int<Twidth,Signed> tmp;
  tmp = in.template slc<Twidth>(0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => AC_FIXED
template<int Twidth, int Ibits, bool Signed, ac_q_mode Qmode, ac_o_mode Omode>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ac_fixed<Twidth,Ibits,Signed,Qmode,Omode> *result)
{
  ac_int<Twidth,Signed> tmp;
  vector_to_type(in, issigned, &tmp);
  result->set_slc(0, tmp);
}
#endif

#if defined(__AC_FLOAT_H) && !defined(MC_TYPECONV_H_AC_FLOAT)
#define MC_TYPECONV_H_AC_FLOAT

#include "ac_sc.h"

// AC_FLOAT => SC_LV
template<int Twidth, int MTbits, int MIbits, int Ebits, ac_q_mode Qmode>
void type_to_vector(const ac_float<MTbits,MIbits,Ebits,Qmode> &in, int length, sc_lv<Twidth> &rvec)
{
  ac_int<MTbits+Ebits,false> tmp = 0;
  tmp.set_slc(0, in.mantissa().template slc<MTbits>(0));
  tmp.set_slc(MTbits, in.exp().template slc<Ebits>(0));
  type_to_vector(tmp, length, rvec);
}

// SC_LV => AC_FLOAT
template<int Twidth, int MTbits, int MIbits, int Ebits, ac_q_mode Qmode>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ac_float<MTbits,MIbits,Ebits,Qmode> *result)
{
  ac_int<MTbits+Ebits,false> tmp;
  ac_int<Ebits,false> tmp_exp = 0;
  ac_fixed<MTbits,MIbits,false> tmp_mantissa;
  vector_to_type(in, issigned, &tmp);
  tmp_exp.set_slc(0, tmp.template slc<Ebits>(MIbits));
  tmp_mantissa.set_slc(0, tmp.template slc<MIbits>(0));
  result->set_mantissa(tmp_mantissa);
  result->set_exp(tmp_exp);
}

#endif

#if defined( __AC_STD_FLOAT_H) && !defined(MC_TYPECONV_H_AC_STD_FLOAT)
#define MC_TYPECONV_H_AC_STD_FLOAT
#include "ac_sc.h"
#define _IEEE_TWIDTH_(_FORMAT_) ((_FORMAT_==binary16) ?16:(_FORMAT_==binary32)?32:(_FORMAT_==binary64)?64:(_FORMAT_==binary128)?128:256 )
#define _IEEE_EWIDTH_(_FORMAT_) ((_FORMAT_==binary16) ?5:(_FORMAT_==binary32)?8:(_FORMAT_==binary64)?11:(_FORMAT_==binary128)?15:19 )

// AC_IEEE_FLOAT => SC_LV
template<ac_ieee_float_format Format>
void type_to_vector(const ac_ieee_float<Format> &in, int length, sc_lv<_IEEE_TWIDTH_(Format)> &rvec)
{
  const int fSize =  _IEEE_TWIDTH_(Format);

  ac_int<fSize,true> tmp = in.data_ac_int();
  type_to_vector(tmp, length, rvec);
}

// SC_LV => AC_IEEE_FLOAT
template<ac_ieee_float_format Format>
void vector_to_type(const sc_lv<_IEEE_TWIDTH_(Format)> &in, bool issigned, ac_ieee_float<Format> *result)
{
  const int fSize = _IEEE_TWIDTH_(Format);
  const int Ebits = _IEEE_EWIDTH_(Format);

  ac_int<fSize,false> tmp;
  vector_to_type(in, issigned, &tmp);
  ac_std_float<fSize, Ebits> stdF;
  stdF.set_data(tmp);
  *result=stdF;
}

// AC_STD_FLOAT => SC_LV
template<int W, int E>
void type_to_vector(const ac_std_float<W,E> &in, int length, sc_lv<W> &rvec)
{
  ac_int<W,true> tmp = in.data();
  type_to_vector(tmp, length, rvec);
}

// SC_LV => AC_STD_FLOAT
template<int W, int E>
void vector_to_type(const sc_lv<W> &in, bool issigned, ac_std_float<W,E> *result)
{
  ac_int<W,false> tmp;
  vector_to_type(in, issigned, &tmp);
  result->set_data(tmp);
}

// bfloat16 => SC_LV
// These need to be inline otherwise we get a link-time name collision
// with questasim mti_ac_types library
template<>
inline void type_to_vector(const ac::bfloat16 &in, int length, sc_lv<16> &rvec)
{
  ac::bfloat16::data_t tmp = in.data();
  type_to_vector(tmp, length, rvec);
}

// SC_LV => bfloat16
template<>
inline void vector_to_type(const sc_lv<16> &in, bool issigned, ac::bfloat16 *result)
{
  const int fSize = 16;

  ac_int<fSize,true> tmp;
  vector_to_type(in, issigned, &tmp);
  result->set_data(tmp);
}

#undef _IEEE_TWIDTH_
#undef _IEEE_EWIDTH_
#endif

#if !defined(MC_TYPECONV_H_AP_INT) && (defined(__AESL_AP_SIM_H__) || defined(__AP_INT_H__))
#define MC_TYPECONV_H_AP_INT

//Explicit conversions from sc_bigint => ap_int and sc_biguint => ap_uint
template<int Twidth>
ap_int<Twidth> to_ap(const sc_dt::sc_bigint<Twidth> &val)
{
  enum { Tnum = (Twidth+31)/32 };
  sc_dt::sc_bigint<Tnum*32> v = val;
  ap_int<Tnum*32> r = 0;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for (int i = 0; i < Tnum; i++) {
    r.range((i+1)*32-1, i*32) = ap_int<32>(v.to_int());
    v >>= 32;
  }
  return ap_int<Twidth>(r);
}
template<int Twidth>
ap_uint<Twidth> to_ap(const sc_dt::sc_biguint<Twidth> &val)
{
  enum { Tnum = (Twidth+31)/32 };
  sc_dt::sc_biguint<Tnum*32> v = val;
  ap_uint<Tnum*32> r = 0;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for (int i = 0; i < Tnum; i++) {
    r.range((i+1)*32-1, i*32) = ap_uint<32>(v.to_int());
    v >>= 32;
  }
  return ap_uint<Twidth>(r);
}
//Explicit conversions from ap_int => sc_bigint and ap_uint => sc_biguint
template <int Twidth>
sc_dt::sc_bigint<Twidth> to_sc(const ap_int<Twidth> &val)
{
  enum { Tnum = (Twidth+31)/32 };
  ap_int<Tnum*32> v = val;
  sc_dt::sc_bigint<Tnum*32> r;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for (int i = Tnum-1; i >= 0; i--) {
    r <<= 32;
    r.range(31, 0) = (v.range((i+1)*32-1, i*32)).to_int();
  }
  return sc_dt::sc_bigint<Twidth>(r);
}
template <int Twidth>
sc_dt::sc_biguint<Twidth> to_sc(const ap_uint<Twidth> &val)
{
  enum { Tnum = (Twidth+31)/32 };
  ap_uint<Tnum*32> v = val;
  sc_dt::sc_biguint<Tnum*32> r;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for (int i = Tnum-1; i >= 0; i--) {
    r <<= 32;
    r.range(31, 0) = (v.range((i+1)*32-1, i*32)).to_int();
  }
  return sc_dt::sc_bigint<Twidth>(r);
}

// AP_INT => SC_LV
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ap_int<Twidth> *result)
{
  sc_bigint<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  *result = to_ap(tmp);
}
template<int Twidth>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ap_uint<Twidth> *result)
{
  sc_biguint<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  *result = to_ap(tmp);
}

// SC_LV => AP_INT
template<int Twidth>
void type_to_vector(const ap_int<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  sc_bigint<Twidth> tmp;
  tmp = to_sc(in);
  type_to_vector(tmp, length, rvec);
}
template<int Twidth>
void type_to_vector(const ap_uint<Twidth> &in, int length, sc_lv<Twidth> &rvec)
{
  sc_biguint<Twidth> tmp;
  tmp = to_sc(in);
  type_to_vector(tmp, length, rvec);
}

// SC_LOGIC => AP_UINT
inline
void vector_to_type(const sc_logic &in, bool issigned, ap_uint<1> *result)
{
  sc_uint<1> tmp;
  vector_to_type(in,false,&tmp);
  *result = tmp.to_uint();
}
// AP_UINT => SC_LOGIC
inline
void type_to_vector(const ap_uint<1> &in, int length, sc_logic &rvec)
{
  sc_uint<1> tmp = in.to_uint();
  type_to_vector(tmp,1,rvec);
}

#if defined(__AESL_AP_SIM_H__) || defined(__AP_FIXED_H__)
// AP_FIXED => SC_LV
template<int Twidth, int Ibits, ap_q_mode Qmode, ap_o_mode Omode, int Sbits>
void type_to_vector(const ap_fixed<Twidth,Ibits,Qmode,Omode,Sbits> &in, int length, sc_lv<Twidth> &rvec)
{
  ap_int<Twidth> tmp;
  tmp.range(Twidth-1, 0) = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}
template<int Twidth, int Ibits, ap_q_mode Qmode, ap_o_mode Omode, int Sbits>
void type_to_vector(const ap_ufixed<Twidth,Ibits,Qmode,Omode,Sbits> &in, int length, sc_lv<Twidth> &rvec)
{
  ap_uint<Twidth> tmp;
  tmp.range(Twidth-1, 0) = in.range(Twidth-1, 0);
  type_to_vector(tmp, length, rvec);
}

// SC_LV => AP_FIXED
template<int Twidth, int Ibits, ap_q_mode Qmode, ap_o_mode Omode, int Sbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ap_fixed<Twidth,Ibits,Qmode,Omode,Sbits> *result)
{
  ap_int<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  result->range(Twidth-1, 0) = tmp.range(Twidth-1, 0);
}
template<int Twidth, int Ibits, ap_q_mode Qmode, ap_o_mode Omode, int Sbits>
void vector_to_type(const sc_lv<Twidth> &in, bool issigned, ap_ufixed<Twidth,Ibits,Qmode,Omode,Sbits> *result)
{
  ap_uint<Twidth> tmp;
  vector_to_type(in, issigned, &tmp);
  result->range(Twidth-1, 0) = tmp.range(Twidth-1, 0);
}
#endif
#endif


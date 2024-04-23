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
// File: mc_container_types.h
//
// Description: Generic 1-dimensional array container class
//
// Revision History:
//    1.2.1 - Initial version on github
//*****************************************************************************************

//////////////////////////////////////////////////////////////////////////////
// mgc_sysc_ver_array1D
//    Generic 1-dimensional array container class
//////////////////////////////////////////////////////////////////////////////
// This class represents the minimal requirements of a 1-D array container
// class which is synthesizable with Catapult C Synthesis and simulatable
// with the Catapult C Synthesis System-C Verification Flow.
// Note that this class does not represent any particular hardware
// implementation. It merely provides an encapsulation of the concept of an
// array with specific operators for simulation.
// Neither does it mandate a particular data type for the array elements.
// Any data type could be used. For example:
//     Builtin C/C++ types such as int, bool, long, float
//     Catapult C Synthesis bit-accurate types such as ac_int and ac_fixed
//     SystemC data types such as sc_int, sc_uint, sc_fixed, sc_ufixed
//     (Not all of these types are synthesizable)
//
// Other implementations of array container classes may be used in place of
// this class as long as they provide the following operators:
//
// Required Operators:
//    operator== (Comparison of one array with another)
//       This operator is required for simulation in the SystemC flow. It
//       enables data in the golden array to be compared with the data array
//       from the synthesized block.
//    operator[] (Read/Write element access)
//       This operator is required so that the testbench environment can
//       modify elements of the data array from the synthesized block.
//    operator[] const (Read-Only element access)
//       This operator is required so that the testbench environment can
//       read values from a data array.
// Optional Methods:
//    operator= (assign one array to another)
//    operator= (assign a single value to all elements)
//       This operator is useful for initializing array contents for
//       more deterministic simulation.
//    operator&= (bit-mask all elements with a single value)
//       This operator is useful for bitmasking output values
//       before comparison.
//    ostream& operator<< (send formated contents to output stream)
//    ostream& operator<< (send formated contents to output stream)
//       This operator is useful for showing the contents of the
//       arrays in diagnostic messages.
//////////////////////////////////////////////////////////////////////////////

#ifndef MC_CONTAINER_TYPES_H
#define MC_CONTAINER_TYPES_H

// Check for macro definitions that will conflict with template parameter names in this file
#if defined(Tclass)
#define Tclass 0
#error The macro name 'Tclass' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tclass' giving the location of the previous definition.
#endif
#if defined(Tsize)
#define Tsize 0
#error The macro name 'Tsize' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tsize' giving the location of the previous definition.
#endif
#if defined(RhsType)
#define RhsType 0
#error The macro name 'RhsType' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'RhsType' giving the location of the previous definition.
#endif
#if defined(Tarrayclass)
#define Tarrayclass 0
#error The macro name 'Tarrayclass' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tarrayclass' giving the location of the previous definition.
#endif
#if defined(Tnsize)
#define Tnsize 0
#error The macro name 'Tnsize' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tnsize' giving the location of the previous definition.
#endif
#if defined(Tvclass)
#define Tvclass 0
#error The macro name 'Tvclass' conflicts with a Template Parameter name.
#error The compiler should have produced a warning about redefinition of 'Tvclass' giving the location of the previous definition.
#endif

#ifndef __SYNTHESIS__
#include <iostream>
#include <sstream>
#endif

// allow use in non-system C env
#ifdef MC_NO_SYSC
#define SC_REPORT_WARNING(id,msg) ::std::cout << msg << ::std::endl;
#endif

#include <cassert>
#include <climits>

template <class Tclass, int Tsize>
class mgc_sysc_ver_array1D
{
  class Data
  {
  public:
    Data() : _cnt(0) {}

    Data(const Data &rhs) : _cnt(0) { operator=(rhs.val); }

    template <class RhsType>
    explicit Data(const RhsType &x) : _cnt(0) { operator=(x); }

    ~Data() { assert(_cnt == 0); }

    Data &operator=(const Data &rhs) { return operator=(rhs.val); }

    Data &operator=(const Tclass &x) {
      for (int i = 0; i < Tsize; ++i) { val[i] = x; }
      return *this;
    }

    template <class Tmask>
    Data &operator&=(const Tmask &x) {
      for (int i = 0; i < Tsize; ++i) { val[i] &= x; }
      return *this;
    }

    Data &operator=(const Tclass a[]) {
      for (int i = 0; i < Tsize; ++i) { val[i] = a[i]; }
      return *this;
    }

    Data &operator=(Tclass a[]) {
      for (int i = 0; i < Tsize; ++i) { val[i] = a[i]; }
      return *this;
    }

    // assigment from multi dimensional arrays Tclass a[D1]...[Dn]
    template <class Tarrayclass>
    Data &operator=(Tarrayclass a[]) {
      Tclass *p = val;
      for (int i = 0; p < val + Tsize; ++i) { assign(a[i], p); }
      return *this;
    }

    Tclass val[Tsize];

    void inc() { assert(_cnt < INT_MAX); ++_cnt; }

    int dec() { assert(_cnt > 0); return --_cnt; }

    int count() const { return _cnt; }
  private:

    template <int Tnsize>
    void assign(Tclass (&a)[Tnsize], Tclass *&p) {
      assert(Tsize % Tnsize == 0);
      assert(val <= p && p + Tnsize <= val + Tsize);
      for (int i = 0; i < Tnsize; ++i) { *p++ = a[i]; }
    }

    template <int Tnsize>
    void assign(const Tclass (&a)[Tnsize], Tclass *&p) {
      assert(Tsize % Tnsize == 0);
      assert(val <= p && p + Tnsize <= val + Tsize);
      for (int i = 0; i < Tnsize; ++i) { *p++ = a[i]; }
    }

    template <class Tarrayclass, int Tnsize>
    void assign(Tarrayclass (&a)[Tnsize], Tclass *&p) {
      for (int i = 0; i < Tnsize; ++i) { assign(a[i], p); }
    }

    int _cnt;
  };

public:
  // initializes all elements to Tclass()
  mgc_sysc_ver_array1D() : _lower(-1), _upper(-1), _p(0), _x(Tclass()) {}

  mgc_sysc_ver_array1D(const mgc_sysc_ver_array1D &rhs) : _lower(rhs._lower), _upper(rhs._upper), _p(rhs._p), _x(rhs._x) {
    if (_p) { _p->inc(); }
  }

  // initializes all elements to value x
  explicit mgc_sysc_ver_array1D(const Tclass &x) : _lower(-1), _upper(-1), _p(0), _x(x) {}

  explicit mgc_sysc_ver_array1D(const Tclass val[]) : _lower(-1), _upper(-1), _p(new Data(val)) {
    _p->inc();
  }

  explicit mgc_sysc_ver_array1D(Tclass val[]) : _lower(-1), _upper(-1), _p(new Data(val)) {
    _p->inc();
  }

  // construction from multi dimensional arrays Tclass a[D1]...[Dn]
  template<class Tarrayclass>
  explicit mgc_sysc_ver_array1D(Tarrayclass val[]) : _lower(-1), _upper(-1), _p(new Data(val)) {
    _p->inc();
  }

  ~mgc_sysc_ver_array1D() { if (_p && _p->dec() == 0) {delete _p; _p = 0;} }

  void set_lower(int lower) { _lower=lower; }
  void set_upper(int upper) { _upper=upper; }
  int get_lower () const { return _lower; }
  int get_upper () const { return _upper; }

  void mc_testbench_process_array_bounds(const char *var, int &first, int &last, unsigned int hi, unsigned int lo) {
    if ( (first != -1) && ( ((unsigned int)first > hi) || ((unsigned int)first < 0) ) ) {
#if !defined(__SYNTHESIS__) && defined(MC_NO_SYSC)
      std::ostringstream msg;
      msg.str("");
      msg << "Testbench control testbench::" << var << "_array_comp_first=" << first << " exceeds array range of [" << lo << "..." << hi << "]";
      SC_REPORT_WARNING("User testbench", msg.str().c_str());
#endif
      first = -1;
    }
    if ( (last != -1) && ( ((unsigned int)last > hi) || ((unsigned int)last < 0) ) ) {
#if !defined(__SYNTHESIS__) && defined(MC_NO_SYSC)
      std::ostringstream msg;
      msg.str("");
      msg << "Testbench control testbench::" << var << "_array_comp_last=" << last << " exceeds array range of [" << lo << "..." << hi << "]";
      SC_REPORT_WARNING("User testbench", msg.str().c_str());
#endif
      last = -1;
    }
    if ( (first != -1) && (last != -1) && (first > last) ) {
#if !defined(__SYNTHESIS__) && defined(MC_NO_SYSC)
      std::ostringstream msg;
      msg.str("");
      msg << "Testbench controls testbench::" << var << "_array_comp_first," << var << "_array_comp_last specify invalid range [" << first << "..." << last << "]";
      SC_REPORT_WARNING("User testbench", msg.str().c_str());
#endif
      first = -1;
      last = -1;
    }
    if (first != -1) { _lower = first; }
    if (last != -1) { _upper = last; }
    first = -1;
    last = -1;
  }

  mgc_sysc_ver_array1D &operator=(const mgc_sysc_ver_array1D &rhs) {
    if (rhs._p) { rhs._p->inc(); }
    if (_p && _p->dec() == 0) { delete _p; }
    _p = rhs._p;
    _x = rhs._x;
    _lower = rhs._lower;
    _upper = rhs._upper;
    return *this;
  }

  // assigns all elements to value x
  mgc_sysc_ver_array1D &operator=(const Tclass &x) {
    if (_p && _p->dec() == 0) { delete _p; }
    _p = 0;
    _x = x;
    return *this;
  }

  template <class Tmask>
  mgc_sysc_ver_array1D &operator&=(const Tmask &mask) {
    if (_p) {
      if (_p->count() > 1) {
        _p->dec();
        _p = new Data(*_p);
        _p->inc();
      }
      assert(_p->count()==1);
      *_p &= mask;
    } else {
      _x &= mask;
    }
    return *this;
  }

  mgc_sysc_ver_array1D &operator=(Tclass rhs[]) { return assign(rhs); }

  mgc_sysc_ver_array1D &operator=(const Tclass rhs[]) { return assign(rhs); }

  // assigment from multi dimensional arrays Tclass a[D1]...[Dn]
  template<class Tarrayclass>
  mgc_sysc_ver_array1D &operator=(Tarrayclass rhs[]) { return assign(rhs); }

  void copy_to(mgc_sysc_ver_array1D &lhs) const { lhs = *this; }

  void copy_to(Tclass lhs[]) const;

  // write to multi dimensional arrays Tclass a[D1]...[Dn]
  template <class Tarrayclass>
  void copy_to(Tarrayclass lhs[]) const;

  bool operator==(const mgc_sysc_ver_array1D &rhs) const;
  bool operator!=(const mgc_sysc_ver_array1D &rhs) const {
    return !operator==(rhs);
  }

  Tclass       &operator[](int i);
  const Tclass &operator[](int i) const {
    if (_p) {
      return _p->val[i];
    } else {
      return _x;
    }
  }

private:
  template <class Tarrayclass>
  mgc_sysc_ver_array1D &assign(Tarrayclass rhs);

  template <int Tnsize>
  void copy_to(Tclass (&lhs)[Tnsize], const Tclass *&) const;

  template <class Tarrayclass, int Tnsize>
  void copy_to(Tarrayclass (&lhs)[Tnsize], const Tclass *&) const;

  template <int Tnsize>
  void copy_to(Tclass (&lhs)[Tnsize], int &) const;

  template <class Tarrayclass, int Tnsize>
  void copy_to(Tarrayclass (&lhs)[Tnsize], int &) const;

  int _lower; // bounds of elements to compare in operator==
  int _upper; // (-1 means full range 0 to Tsize)

  Data *_p; // If _p == 0, behaves as if all elements were equal
  Tclass     _x; // to _x. If _p != 0, then _x has no significance.
};

////////////////////////////////////////////////////////////////////////////////
// member function defintions
////////////////////////////////////////////////////////////////////////////////

template<class Tclass, int Tsize>
bool mgc_sysc_ver_array1D<Tclass,Tsize>::operator==(const mgc_sysc_ver_array1D &rhs) const
{
  int LB = (_lower < 0) ? 0       : _lower;
  int UB = (_upper < 0) ? Tsize-1 : _upper;
  assert(LB < Tsize);
  assert(UB <= Tsize-1);
  if (_p == 0) {
    if (rhs._p == 0) {
      return _x == rhs._x;
    } else {
      for (int i = LB; i <= UB; ++i) {
        if (rhs._p->val[i] != _x) { return false; }
      }
      return true;
    }
  } else if (rhs._p == 0) {
    for (int i = LB; i <= UB; ++i) {
      if (_p->val[i] != rhs._x) { return false; }
    }
    return true;
  } else {
    if (_p == rhs._p) { return true; }
    for (int i = LB; i <= UB; ++i) {
      if (rhs._p->val[i] != _p->val[i]) { return false; }
    }
    return true;
  }
}

template <class Tclass, int Tsize>
template <class Tvclass>
mgc_sysc_ver_array1D<Tclass,Tsize> &mgc_sysc_ver_array1D<Tclass,Tsize>::assign(Tvclass rhs)
{
  if (_p) {
    if (_p->count() == 1) {
      *_p = rhs;
      return *this;
    } else {
      _p->dec();
    }
  }
  _p = new Data(rhs);
  _p->inc();
  return *this;
}


template<class Tclass, int Tsize>
template<int Tnsize>
inline void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tclass (&lhs)[Tnsize], const Tclass *&p) const
{
  assert(Tsize % Tnsize == 0);
  assert(_p->val <= p && p + Tnsize <= _p->val + Tsize);
  for (int i = 0; i < Tnsize; ++i) { lhs[i] = *p++; }
}

template<class Tclass, int Tsize>
template<class Tarrayclass, int Tnsize>
inline void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tarrayclass (&lhs)[Tnsize], const Tclass *&p) const
{
  for (int i = 0; i < Tnsize; ++i) { copy_to(lhs[i], p); }
}

template<class Tclass, int Tsize>
template<int Tnsize>
inline void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tclass (&lhs)[Tnsize], int &j) const
{
  assert(Tsize % Tnsize == 0);
  for (int i = 0; i < Tnsize; ++i) { lhs[i] = _x; }
  j += Tnsize;
}

template<class Tclass, int Tsize>
template<class Tarrayclass, int Tnsize>
inline void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tarrayclass (&lhs)[Tnsize], int &j) const
{
  for (int i = 0; i < Tnsize; ++i) { copy_to(lhs[i], j); }
}

template<class Tclass, int Tsize>
void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tclass lhs[]) const
{
  if (_p) {
    for (int i = 0; i < Tsize; ++i) { lhs[i] = _p->val[i]; }
  } else {
    for (int i = 0; i < Tsize; ++i) { lhs[i] = _x; }
  }
}

template<class Tclass, int Tsize>
template<class Tarrayclass>
void mgc_sysc_ver_array1D<Tclass,Tsize>::copy_to(Tarrayclass lhs[]) const
{
  if (_p) {
    const Tclass *p = _p->val;
    for (int i = 0; p < _p->val + Tsize; ++i) { copy_to(lhs[i], p); }
  } else {
    int j = 0;
    for (int i = 0; j < Tsize; ++i) { copy_to(lhs[i], j); }
  }
}

template<class Tclass, int Tsize>
Tclass &mgc_sysc_ver_array1D<Tclass,Tsize>::operator[](int i)
{
  if (_p == 0) {
    _p = new Data(_x);
    _p->inc();
  } else if (_p->count() > 1) {
    Data *p = new Data(*_p);
    _p->dec();
    _p = p;
    _p->inc();
  }

  return _p->val[i];
}


#ifndef __SYNTHESIS__
template<class Tclass, int Tsize>
std::ostream &operator<<(std::ostream &os, const mgc_sysc_ver_array1D<Tclass,Tsize> &a)
{
  os << '{';
  for (int i = 0; i < Tsize; ++i) { os << a[i] << ' '; }
  os << '}';
  return os;
}
#endif

template <class _Tclass>
struct mc_container_type_traits {
  enum {Tsize = 1};
  typedef _Tclass Tclass;
};

template <class _Tclass, int _Tsize>
struct mc_container_type_traits< mgc_sysc_ver_array1D<_Tclass,_Tsize> > {
  enum {Tsize = _Tsize};
  typedef _Tclass Tclass;
};

// end of MC_CONTAINER_TYPES_H
#endif


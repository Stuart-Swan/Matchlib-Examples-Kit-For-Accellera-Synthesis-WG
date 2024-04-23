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
#ifndef _INCLUDE_AC_SYSC_TRACE_H_
#define _INCLUDE_AC_SYSC_TRACE_H_

//
// ac_sysc_trace.h
//
// Helpful macros and functionality for simulation trace SystemC designs.
//

#include <systemc.h>

//
// The SC_SIG, SC_VAR, and SC_VAR_NM  macros assume a mechanism to get an open sc_trace_file
// This is the mechanism pto provide that
// See: https://stackoverflow.com/questions/18860895/how-to-initialize-static-members-in-the-header
//
// In sc_main, do this:
// sc_trace_file *trace_file = sc_trace_static::setup_trace_file("trace");
//
class sc_trace_static
{
public:
  sc_trace_static() {}
  ~sc_trace_static() {(void) static_accessor(0,true);}

  static sc_trace_file *static_accessor(sc_trace_file *newPtr = 0, bool done=false) {
    static sc_trace_file *staticPtr = 0;
    if (newPtr) {
      staticPtr = newPtr;
    }
    if (done && staticPtr) {
      sc_close_vcd_trace_file(staticPtr);
    }

    return (staticPtr);
  }

  static sc_trace_file *setup_trace_file(const char *fname) {
    sc_trace_file *retPtr = static_accessor();
    if (retPtr) {
      std::cout << "Error: Can only call CCS_TraceFileStatics::setup_trace_file once.  Original file used!" << std::endl;
    } else {
      if (fname) {
        retPtr = sc_create_vcd_trace_file(fname);
      } else {
        retPtr = sc_create_vcd_trace_file("trace");
      }
      (void) static_accessor(retPtr);
    }
    return (retPtr);
  }
};

#ifndef _CCS_OLD_CONNECTIONS_
//
// sc_trace helpers used when the below macros are used to declared variables
// and signals which are to be traced.
template <class T>
struct sc_object_tracer {
  sc_object_tracer(T &obj) {
    sc_trace(sc_trace_static::static_accessor(), obj, obj.name());
  }
  ~sc_object_tracer() {}
};

template <class T>
struct sc_var_tracer {
  sc_var_tracer(T &obj, const std::string &nm)  {
    sc_trace(sc_trace_static::static_accessor(), obj, nm);
  }
  ~sc_var_tracer() {}
};

#else
// Hardwired for this global fileptr
extern sc_trace_file *trace_file_ptr;
template <class T>
struct sc_object_tracer {
  sc_object_tracer(T &obj) {
    sc_trace(trace_file_ptr, obj, obj.name());
  }
  ~sc_object_tracer() {}
};

template <class T>
struct sc_var_tracer {
  sc_var_tracer(T &obj, const std::string &nm)  {
    sc_trace(trace_file_ptr, obj, nm);
  }
  ~sc_var_tracer() {}
};
#endif

// Macro: SC_SIG(T,N)
//  Useful for declaring an sc_signal of type T with name N and cause it to be traced.
//
// Macro:  SC_VAR(T,N)
//  Useful for declaring avariable of type T with name N and cause it to be traced.
//
// Macro:  SC_VARNM(T,N,NM)
//  Useful for declaring a variable of type T with name N and cause it to be traced.
//  The name used for tracing will use scope NM for its name.

#if (defined(SC_SIG) || defined(SC_VAR) || defined(SC_VAR_NM))
#warning One or more of the following is defined: SC_SIG.SC_VAR,SC_VAR_NM. Definition conflicts with their usage as Systemc helpers.
#warning Unpredictable systemc simulation may result.
#else
#ifndef __SYNTHESIS__
#define SC_SIG(T,N) sc_signal<T> N{#N}  ; sc_object_tracer<sc_signal<T>> N ## _tracer { N }
#define SC_VAR(T,N) T N ; sc_var_tracer<T> N ## _tracer { N, std::string(name()) + std::string(".") + #N }
#define SC_VAR_NM(T,N, NM) T N ; sc_var_tracer<T> N ## _tracer { N, std::string(NM) + std::string(".") + #N }
#else
#define SC_SIG(T,N) sc_signal<T> N{#N}
#define SC_VAR(T,N) T N
#define SC_VAR_NM(T,N, NM) T N
#endif
#endif


/**
 * Example of how to enable tracing for user defined structs:
 * The inline function is of interest.
 *

struct MyType
{
    int info {0};
    bool flag {false};

    inline friend void sc_trace(sc_trace_file *tf, const MyType & v, const std::string& NAME ) {
      sc_trace(tf,v.info, NAME + ".info");
      sc_trace(tf,v.flag, NAME + ".flag");
    }
};

**/


#endif  // _INCLUDE_AC_SYSC_TRACE_H_ 

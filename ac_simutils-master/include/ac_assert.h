/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Simulation Utilities                               *
 *                                                                        *
 *  Software Version: 1.8                                                 *
 *                                                                        *
 *  Release Date    : Thu Jul 24 13:23:36 PDT 2025                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.8.0                                               *
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
// File: ac_assert.h
//
// Description: Provides synthesizable "assert" and "cover" functions.
//      When run with standalone C++ execution, at the end of execution a primative
//      coverage report is dumped to the console.
//
// Assertion macros/behavior
// 
// Macro           | Msg at | Total | Abort   | Synth  | Msg at | Total | Abort   | Fail |
//                 | fire   | msgs? | or Cont | to RTL | fire   | msgs  | or Cont |      |
// ---------------------------------------------------------------------------------------
// AC_ASSERTION(X) | 1 msg  | n/a   | abort   | SVA    | 1 msg  | yes   | cont    | yes  |
// HLS_ASSERTION(x)|        |       |         |        |        |       |         |      |
// ---------------------------------------------------------------------------------------
// RTL_ASSERTION(X)| no     | no    | cont    | SVA    | 1 msg  | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------
// softassert(X)   | 1 msg  | yes   | cont    | SVA    | 1 msg  | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------
// assert(X)       | 1 msg  | n/a   | abort   | no     | 1 msg  | n/a   | abort   | n/a  |
//  after 2025.3   |        |       |         |        |        |       |         |      |
// ---------------------------------------------------------------------------------------
// assert(X) when  | 1 msg  | n/a   | abort   | SVA    | 1 msg  | yes   | cont    | yes  |
// AC_ASSERT_REDEF |        |       |         |        |        |       |         |      |
// defined         |        |       |         |        |        |       |         |      |
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// ovl_assert(X)   | 1 msg  | yes   | cont    | OVL    | 1 msg  | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------
// AC_OVL_ASSERT(X)| 1 msg  | yes   | cont    | OVL    | 1 msg  | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// sc_assert(X)    | 1 msg  | n/a   | abort   | SVA    | 1 msg  | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------
// ovl_sc_assert(X)| all    | yes   | cont    | OVL    | all    | yes   | cont    | yes  |
// ---------------------------------------------------------------------------------------
// AC_OVL_SC_      | all    | yes   | cont    | OVL    | all    | yes   | cont    | yes  |
//     ASSERT(X)   |                                                                     |
// ---------------------------------------------------------------------------------------


// Cover macros/behavior
//
// cover(X) not defined
//  after 2025.3
// ovl_cover(X)
// AC_COVER(X)
// HLS_COVER(X)
// AC_OVL_COVER(X)
// HLS_OVL_COVER(X)


//
// Revision History:
//    1.2.0 - Initial version on github
//*****************************************************************************************

#ifndef __AC_ASSERT_H__
#define __AC_ASSERT_H__

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

// if QuestaSim enhanced datatype viewing is enabled, include ac_int to get proper namespace scoping
#ifdef SC_INCLUDE_MTI_AC
#include <ac_int.h>
#endif


#if defined(_WIN32)
#if !defined(_MSC_VER)
#warning Call-stack tracking and end-of-execution cover() summary is not available on non-Microsoft Compilers
#else
#if (_MSC_VER < 1400) || (NTDDI_VERSION <= NTDDI_WINXP)
#pragma message("Call-stack tracking and end-of-execution cover() summary requires Visual Studio 8 or newer")
#else
#define AC_COVER_USES_STACK
#endif
#endif
#else
#define AC_COVER_USES_STACK
#endif

#ifdef AC_COVER_USES_STACK
#if defined(_WIN32)
#include <windows.h>
#include <WinNT.h>
#else
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif
#endif

#include <stdio.h>
#ifndef __ASSERT_H__
#define __ASSERT_H__
#include <assert.h>
#endif

#ifndef __SYNTHESIS__
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <stdarg.h>
#endif

#if defined(CCS_SCVERIFY)
extern void mc_testbench_ac_assert_fail();
#endif

// Define a version macro for backward compatibility if required
#define AC_ASSERT_VER 2

// For backward compatibility with pre-2025.3 releases:
// (this will be removed in a future release but can be manually supplied by the end user at that time)
#define AC_COVER_REDEF 1

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

namespace ac {
  #if !defined(__SYNTHESIS__)

  // Function: build_stack_key
  // Returns a string built from the concatenation of the call stack addresses
  inline void build_stack_key(std::stringstream &key, const int starting_frame=0)
  {
      #ifdef AC_COVER_USES_STACK
      void *buffer[100]; // max stack size
      char ptrstr[17];
      #if defined(_WIN32)
      unsigned short nptrs = CaptureStackBackTrace(0,100,buffer,NULL);
      #else
      int nptrs = backtrace(buffer,100)-1; // always drop outermost frame
      #endif
      if (nptrs > 0) {
        key << " STACK[";
        for (int j=starting_frame;j<nptrs;j++) { sprintf(ptrstr,"%p",(void*)buffer[j]); key << std::string(ptrstr); if (j<nptrs-1) key << ",";}
        key << "]:";
      }
      #endif
  }

  // Manage assertion triggers
  class assert_manager : public std::map<std::string,int>
  {
  public:
    assert_manager() {}
    ~assert_manager() {
      if (!empty()) {
        for (std::map<std::string,int>::iterator pos=begin(); pos!=end(); ++pos) {
          if ( (*pos).second > 0) {
            std::string key((*pos).first);
            std::size_t endstack = 0;
            endstack = key.find_first_of(":")+1;
            bool quiet = (key[endstack] == 'T');
#if defined(CCS_SCVERIFY)
            (void)quiet;
            printf("Warning: ac_assert: %s: ASSERT TRIGGERED %d TIME(S)\n", key.substr(endstack+1).c_str(), (*pos).second);
#else
            // RTL_ASSERTION will not print summary counts during C++ execution
            if (!quiet) {
              printf("%s: ASSERT TRIGGERED %d TIME(S)\n", key.substr(endstack+1).c_str(), (*pos).second);
            }
#endif

          }
        }
      }
    }
    // returns true if message should be reported
    bool check_assert(const char *filename, int lineno, const char *expr_str, bool expr, bool quiet=false) {
      if (!expr) {
        std::stringstream key;
        build_stack_key(key,0);
        key << (quiet?"T":"F") << filename << ": " << lineno << " {" << expr_str << "}";
        std::map<std::string,int>::iterator pos = find(key.str());
        if (pos == end()) {
          insert(std::pair<std::string,int>(key.str(),0));
          pos = find(key.str());
        }
        (*pos).second++; // increment count in entry
        if ( (*pos).second <= 1) return true;
      }
      return false;
    }
  };

  // Manage cover directives "ac_cover"
  class cover_manager : public std::map<std::string,int>
  {
  public:
    cover_manager() {}
    ~cover_manager() {
      if (!empty()) {
        for (std::map<std::string,int>::iterator pos=begin(); pos!=end(); ++pos) {
          if ( (*pos).second == 0) {
            std::string key((*pos).first);
            std::size_t endstack = 0;
            #ifdef AC_COVER_USES_STACK
            endstack = key.find_first_of(":")+1;
            #endif
            std::size_t brace = key.find_first_of("{",endstack);
            printf("%s: MISSED COVERAGE\n", key.substr(endstack,brace-1).c_str());
          }
        }
      } else {
        printf("WARNING - NO COVER CALLS EXECUTED\n");
      }
    }
    // returns true if message should be reported
    bool cover(const char *filename, int lineno, const char *expr_str, bool expr) {
      std::stringstream key;
      build_stack_key(key,0);
      key << filename << ": " << lineno << " {" << expr_str << "}";
      std::map<std::string,int>::iterator pos = find(key.str());
      if (pos == end()) {
        insert(std::pair<std::string,int>(key.str(),0));
        pos = find(key.str());
      }
      if (expr) {
        (*pos).second++; // increment count in entry
        if ( (*pos).second <= 1) return true;
      }
      return false;
    }
  };

#endif

  // Synthesizable assert "ac_assert"
  inline void ac_assert(const char *filename, int lineno, const char *expr_str, bool expr
#if !defined(__SYNTHESIS__)
  , bool quiet=false
#endif
  ) {
#if defined(__SYNTHESIS__)
    // builtin function
#else
    static assert_manager s_am;
    if (s_am.check_assert(filename,lineno,expr_str,expr,quiet)) {
#if defined(CCS_SCVERIFY)
        // print message a message then set failure flag for end of SCVerify
        printf("Warning: ac_assert: %s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
        mc_testbench_ac_assert_fail();
#else
        if (!quiet) {
          printf("%s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
        }
#endif
    }
#endif // endif __SYNTHESIS__
  }

  // Synthesizable assert "ac_ovl_assert"
  inline void ac_ovl_assert(const char *filename, int lineno, const char *expr_str, bool expr
#ifndef CALYPTO_SC
  , bool quiet=false
#endif
  ) {
#if defined(__SYNTHESIS__)
    // builtin function
#else
    static assert_manager s_am;
    if (s_am.check_assert(filename,lineno,expr_str,expr)) {
#if defined(CCS_SCVERIFY)
        // print message a message then set failure flag for end of SCVerify
        printf("Warning: ac_assert: %s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
        mc_testbench_ac_assert_fail();
#else
        if (!quiet) {
          printf("%s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
        }
#endif
    }
#endif // endif __SYNTHESIS__
  }

  // Synthesizable cover directive "ac_cover"
  inline void ac_cover(const char *filename, int lineno, const char *expr_str, bool expr) {
    #if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    static cover_manager s_cm;
    if (s_cm.cover(filename,lineno,expr_str,expr)) {
      printf("%s: %d: %s: COVERAGE ENCOUNTERED\n", filename, lineno, expr_str);
    }
    #endif // endif __SYNTHESIS__
  }

  // Synthesizable cover directive "ac_ovl_cover"
  inline void ac_ovl_cover(const char *filename, int lineno, const char *expr_str, bool expr) {
    #if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    static cover_manager s_cm;
    if (s_cm.cover(filename,lineno,expr_str,expr)) {
      printf("%s: %d: %s: COVERAGE ENCOUNTERED\n", filename, lineno, expr_str);
    }
    #endif // endif __SYNTHESIS__
  }

} // end namespace ac

#ifdef __AC_NAMESPACE
}
#endif

// ASSERTION SYNTHESIS
// Replace macros during SYNTHESIS and SCVERIFY
// Leave original simulation untouched
#if defined(CCS_SCVERIFY) || defined(__SYNTHESIS__)
  // If not SLEC
  #ifndef CALYPTO_SC
    // assert synthesized to PSL property
    #ifdef sc_assert
      #undef sc_assert
    #endif
    #define sc_assert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
  #endif // CALYPTO_SC
#endif // CCS_SCVERIFY || __SYNTHESIS__

// Convenience macros
#if defined(CCS_SCVERIFY) || defined(__SYNTHESIS__)
#define AC_ASSERTION(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#define HLS_ASSERTION(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#else
#define AC_ASSERTION(expr) assert(expr)
#define HLS_ASSERTION(expr) assert(expr)
#endif

// assert synthesized to OVL instance
#define ovl_assert(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)
#define ovl_sc_assert(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)
#define AC_OVL_ASSERT(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)
#define AC_OVL_SC_ASSERT(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)

// Use softassert() instead of assert() if you do not want your C++ program to abort
#define softassert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#if defined(__SYNTHESIS__)
#define RTL_ASSERTION(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#else
#define RTL_ASSERTION(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr, true)
#endif

// COVER DIRECTIVE SYNTHESIS
#if defined(AC_COVER_REDEF)
// cover synthesized to PSL property
#define cover(expr) ac::ac_cover(__FILE__, __LINE__, #expr, expr)
// cover synthesized to OVL instance
#define ovl_cover(expr) ac::ac_ovl_cover(__FILE__, __LINE__, #expr, expr)
#endif
// cover synthesized to PSL property
#define AC_COVER(expr) ac::ac_cover(__FILE__, __LINE__, #expr, expr)
#define HLS_COVER(expr) ac::ac_cover(__FILE__, __LINE__, #expr, expr)
// cover synthesized to OVL instance
#define AC_OVL_COVER(expr) ac::ac_ovl_cover(__FILE__, __LINE__, #expr, expr)
#define HLS_OVL_COVER(expr) ac::ac_ovl_cover(__FILE__, __LINE__, #expr, expr)

// This marks the end of the portion of this header that is guarded by
// include-fencing. The remainder of this file is to (re)define the
// assert macros that the standard assert.h will blindly redefine if it
// happens to get included later. This ac_assert.h header will then be
// forced to be the last header included by any C++ file generated by Catapult.
#endif // endif __AC_ASSERT_H__

// ASSERTION SYNTHESIS
// Replace macros during SYNTHESIS and SCVERIFY
// Leave original simulation untouched
#if defined(CCS_SCVERIFY) || defined(__SYNTHESIS__)
  // If not SLEC
  #ifndef CALYPTO_SC
    #if defined(AC_ASSERT_REDEF)
    // assert synthesized to PSL property
    #undef assert
    #define assert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
    #else
    #undef assert
    #define assert(expr)
    #endif
  #endif // CALYPTO_SC
#endif // CCS_SCVERIFY || __SYNTHESIS__


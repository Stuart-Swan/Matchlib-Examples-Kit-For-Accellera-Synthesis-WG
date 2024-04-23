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
// File: ac_assert.h
//
// Description: Provides synthesizable "assert" and "cover" functions.
//      When run with standalone C++ execution, at the end of execution a primative
//      coverage report is dumped to the console.
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


#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

namespace ac {
  #if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)

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
  inline void ac_assert(const char *filename, int lineno, const char *expr_str, bool expr) {
    #if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    static std::map<std::string,int> s_hit_count;
    if ( ! expr ) {
      std::stringstream key;
      key << filename << ":" << lineno << "{" << expr_str << "}";
      std::map<std::string,int>::iterator pos = s_hit_count.find(key.str());
      if (pos == s_hit_count.end()) {
        s_hit_count.insert(std::pair<std::string,int>(key.str(),0));
      }
      pos = s_hit_count.find(key.str());
      (*pos).second++; // increment count in entry
      if ( (*pos).second <= 1 ) {
        printf("%s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
      }
    }
    #endif // endif __SYNTHESIS__
  }

  // Synthesizable assert "ac_ovl_assert"
  inline void ac_ovl_assert(const char *filename, int lineno, const char *expr_str, bool expr) {
    #if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    static std::map<std::string,int> s_hit_count;
    if ( ! expr ) {
      std::stringstream key;
      key << filename << ":" << lineno << "{" << expr_str << "}";
      std::map<std::string,int>::iterator pos = s_hit_count.find(key.str());
      if (pos == s_hit_count.end()) {
        s_hit_count.insert(std::pair<std::string,int>(key.str(),0));
      }
      pos = s_hit_count.find(key.str());
      (*pos).second++; // increment count in entry
      if ( (*pos).second <= 1 ) {
        printf("%s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
      }
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

// assert synthesized to OVL instance
#define ovl_assert(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)
#define ovl_sc_assert(expr) ac::ac_ovl_assert(__FILE__, __LINE__, #expr, expr)
// Use softassert() instead of assert() if you do not want your C++ program to abort
#define softassert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)

// COVER DIRECTIVE SYNTHESIS
// cover synthesized to PSL property
#define cover(expr) ac::ac_cover(__FILE__, __LINE__, #expr, expr)
// cover synthesized to OVL instance
#define ovl_cover(expr) ac::ac_ovl_cover(__FILE__, __LINE__, #expr, expr)

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
    // assert synthesized to PSL property
    #undef assert
    #define assert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
  #endif // CALYPTO_SC
#endif // CCS_SCVERIFY || __SYNTHESIS__


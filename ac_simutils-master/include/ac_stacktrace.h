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
 *  Copyright 2022 Siemens                                                *
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
// File: ac_stacktrace.h
//
// Description: Provides debug code to build and dump stack traces for use in debugging
//      pre-HLS runtime execution and/or instrumentation.
//
// Example:
//   // Compile using: g++ -std=c++11 -g -I$MGC_HOME/shared/include st.cpp  -lbfd
//
//   #include <ac_stacktrace.h>
//   #include <iostream>
//   
//   void func(int a) {
//     std::stringstream key;
//     ac_debug::build_stack_key(key,1);
//     std::cout << "Call stack ptrs: " << key.str() << std::endl;
//   
//     std::string pkey(key.str());
//     std::regex reg(",");
//     std::sregex_token_iterator iter(pkey.begin(), pkey.end(), reg, -1);
//     std::sregex_token_iterator end;
//     std::vector<std::string> vec(iter, end);
//     for (auto a : vec) {
//       std::stringstream info;
//       ac_debug::addr2fileLineFunc((char*)a.c_str(), info);
//       std::cout << "    " << a << ": " << info.str() << std::endl;
//     }
//   }
//   
//   int main() {
//     func(6);
//     return 0;
//   }
//
//
//
// Revision History:
//    1.2.0 - Initial version on github
//*****************************************************************************************

#ifndef __AC_STACKTRACE_H__
#define __AC_STACKTRACE_H__

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <regex>
#include <utility>
#include <stdarg.h>
#if defined(_WIN32)
// define NOMINMAX to prevent windows.h from defining macros that mess with std max
#define NOMINMAX
#include <windows.h>
#include <WinNT.h>
#else
#include <unistd.h>
#include <execinfo.h>
#include <bfd.h>
#define HAVE_DECL_BASENAME 1
#include <demangle.h>
#endif

#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
//  x86, x86-64 - The program counter (PC) refers to the instruction 
//      after the current executed.
//      As x86 is a CISC architecture, the offset to start of the previous 
//      instruction address is variable.  However, subtracting one (1) 
//      will effectively land the address within the range of the 
//      previous instruction, which is sufficient for retrieving
//      the correct line information.
//    -> subtract 1 from the offset
# define ARCH_PC_OFFSET 1
#else
# warning Functionality only tested for x86 architectures; backtrace information may not be accurate
// For reference only:
//  Arm - AArch32 - see description of program counter in section 
//      "The AArch32 Application Level Programmers' Model" 
//      of "Arm Architecture Reference Manual for A-profile architecture"
//    - A32 instruction -> PC = current instruction + 8 bytes
//    - T32 instruction -> PC = current instruction + 4 bytes
//  RISC-V - The PC refers to the current instruction being executed.
//        See "The RISC-V Instruction Set Manual"
# define ARCH_PC_OFFSET 0
#endif

namespace ac_debug
{

  // Function: build_stack_key
  // Returns a string built from the concatenation of the call stack addresses
  inline void build_stack_key(std::stringstream &key, const int starting_frame=0)
  {
    void *buffer[100]; // max stack size
    char ptrstr[20];
    #if defined(_WIN32)
    unsigned short nptrs = CaptureStackBackTrace(0,62,buffer,NULL);
    #else
    int nptrs = backtrace(buffer,100)-2; // drop 2 outermost frames (i.e. above main) for linux CRT
    #endif
    if (nptrs > 0) {
      for (int j=starting_frame;j<nptrs;j++) { sprintf(ptrstr,"%p",(void*)buffer[j]); key << std::string(ptrstr); if (j<nptrs-1) key << ",";}
    }
  }

#if !defined(_WIN32)
  // Using BFD for locating file/line info
  static bfd* abfd = 0;
  static asymbol **syms = 0;
  static asection *text = 0;
#endif
  static void addr2fileLineFunc(char *address, std::stringstream &info, bool optFormat=false, bool ignoreNoDebugInfo = false) {
    info.clear();
    #if defined(_WIN32)
    info << "(" << address << ") no debug info";
    return;
    #else
    if (!abfd) {
      char ename[1024];
      int l = readlink("/proc/self/exe",ename,sizeof(ename));
      if (l == -1) {
        // could not locate path to executable
        return;
      }
      ename[l] = 0;
      bfd_init();
      abfd = bfd_openr(ename, 0);
      if (!abfd) {
        // could not open binary
        return;
      }
      bfd_check_format(abfd,bfd_object);
      unsigned storage_needed = bfd_get_symtab_upper_bound(abfd);
      syms = (asymbol**)malloc(storage_needed);
      unsigned cSymbols = bfd_canonicalize_symtab(abfd, syms); (void)cSymbols;
      text = bfd_get_section_by_name(abfd, ".text");
    }

    long offset = strtol(address,NULL,16) - text->vma - ARCH_PC_OFFSET;

    if (offset > 0) {
      const char *file;
      const char *func;
      unsigned line;
      if (bfd_find_nearest_line(abfd, text, syms, offset, &file, &func, &line) && file) {
        char *func_demangled = bfd_demangle(abfd, func, DMGL_ANSI | DMGL_PARAMS);
        if (func_demangled != nullptr)
          func = func_demangled;
        if (optFormat) {
          // LeafFile:Line FileDir 'Func'
          std::string str(file);
          std::regex e("(.*)/(.*)");
          std::smatch sm;
          std::regex_match(str.cbegin(), str.cend(), sm, e);
          if (sm.size()==3) {
            info << sm[2] << ":" << line << " '" << func << "' " << file << ":" << line;
          } else {
            // fall back on old form
            info << file << ":" << line << " '" << func << "'";
          }
        } else {
          // File:Line 'Func'
          info << file << ":" << line << " '" << func << "'";
        }
        free(func_demangled);
      } else {
        if (!ignoreNoDebugInfo)
          info << "(" << address << ") no debug info";
      }
    }
    #endif
  }

  // Function: format_stack_trace
  // Uses BFD addr2line to format a nice stack trace
  // csvFormat quotes each item in flat output because the demangled name can contain commas
  inline std::string format_stack_trace(const std::string key, bool flat=false, bool optFormat=false, bool csvFormat=false, bool ignoreNoDebugInfo = false) {
    std::stringstream trace;
    std::regex reg(",");
    std::string pkey(key);
    std::sregex_token_iterator iter(pkey.begin(), pkey.end(), reg, -1);
    std::sregex_token_iterator end;
    std::vector<std::string> vec(iter, end);
    bool first = true;
    for (auto a : vec) {
      std::stringstream info;
      addr2fileLineFunc((char*)a.c_str(), info, optFormat, ignoreNoDebugInfo);
      if (info.str().size() > 0) {
        if (flat) {
          trace << (first ? "" : csvFormat ? "," : ", ") << (csvFormat ? "\"" : "") << info.str() << (csvFormat ? "\"" : "") ;
        } else {
          trace << "    "  << info.str() << std::endl;
        }
        first = false;
      }
    }
    return trace.str();
  }

}

#undef ARCH_PC_OFFSET
#endif


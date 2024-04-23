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
// This file contains functions that can be called from the GDB prompt during debug.

// Use this to print out help text for these functions
__attribute__ ((noinline)) void ac_dbg_help() { 
  std::cout << "AC SystemC Debug Helper Functions:" << std::endl;
  std::cout << "  ac_dbg_time()                      - prints the raw SystemC simulation timestamp" << std::endl;
  std::cout << "  ac_dbg_time_ns()                   - prints the SystemC simulation timestamp in nanoseconds" << std::endl;
  std::cout << "  ac_dbg_name(void *obj)             - prints out the SystemC object's name" << std::endl;
  std::cout << "  ac_dbg_name_is(void *obj, char *s) - use in a conditional breakpoint to stop when a SystemC object has a given name" << std::endl;
}

// Use this to print out the raw SystemC simulation timestamp
__attribute__ ((noinline)) void ac_dbg_time() { std::cout << sc_time_stamp() << "\n"; }

// Use this to print out the SystemC simulation timestamp in nanoseconds
__attribute__ ((noinline)) int  ac_dbg_time_ns() { return sc_time_stamp() / sc_time(1, SC_NS); }

// Use this to print out the SystemC object's name
__attribute__ ((noinline)) void ac_dbg_name(void *obj) { std::cout << ((sc_object *)obj)->name() << "\n"; }

// Use this in a conditional breakpoint to stop when a SystemC object has a given name
__attribute__ ((noinline)) bool ac_dbg_name_is(void *obj, char *s) { return strcmp(((sc_object *)obj)->name(), s) == 0; }


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
#ifndef _INCLUDE_AC_SYSC_MACROS_H_
#define _INCLUDE_AC_SYSC_MACROS_H_

#include <systemc.h>

//
// ac_sysc_macros.h
//
// Helpful macros for coding SystemC designs.
//

// Macro: CCS_INIT_S1(n)
//  Useful for declaring a port constructor with the port name.
//
// Usage:
//   sc_in<type>  CCS_INIT_S1(din);
//
// Results in:
//   din{"din"};
//
#define CCS_INIT_S1(n)              n{#n}
#define CCS_INIT_S2(n, a2)          n{#n, a2}


// Macro: CCS_LOG(str)
//  Useful for logging text to cout in the context where simulation time
//  and object name() are valid and desired to be in the displayed message.
//
// Usage:
//     CCS_LOG("dma_done detected. " << d0 << " " << d1);
// results in:
//     # 4781 ns top dma_done detected. 1 1
//
#ifndef __SYNTHESIS__
#define CCS_LOG(x)  std::cout << sc_time_stamp() << " " << name() << " " << x << std::endl
#else
#define CCS_LOG(x)  // empty
#endif

#ifdef _CCS_OLD_CONNECTIONS_
#define CTOR_S1(n)              n(#n)
#define CTOR_S2(n, a2)          n(#n, a2)
#define CTOR_S3(n, a2, a3)      n(#n, a2, a3)
#define CTOR_S4(n, a2, a3, a4)  n(#n, a2, a3, a4)
#define INIT_S1(n)              n{#n}
#define INIT_S2(n, a2)          n{#n, a2}

#ifndef __SYNTHESIS__
#define LOG(x)  std::cout << sc_time_stamp() << " " << name() << " " << x << std::endl
#else
#define LOG(x)  // empty
#endif

#endif


#endif  // _INCLUDE_AC_SYSC_MACROS_H_ 

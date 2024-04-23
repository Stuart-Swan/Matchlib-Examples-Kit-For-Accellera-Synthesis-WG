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
// File: ac_read_env.h
//
// Description: Provides routines to query the process environment to extract values
//
// Revision History:
//    1.2.0 - Initial version
//*****************************************************************************************

#ifndef _INCLUDE_AC_READ_ENV_H_
#define _INCLUDE_AC_READ_ENV_H_

#if !(__cplusplus >= 201103L)
#warning Please use C++11 for safer string handling
#define AC_READ_ENV_UNSAFE
#endif

#include <string>

namespace ac_env
{
  inline int read_int(const char *optionName, int defaultValue = 0)
  {
    char *value = getenv(optionName);
    if (!value) { return defaultValue; }
    std::string opt(value);
#ifdef AC_READ_ENV_UNSAFE
    return atoi(opt.c_str());
#else
    return std::stoi(opt);
#endif
  }
  inline bool read_bool(const char *optionName, bool defaultValue = false)
  {
    char *value = getenv(optionName);
    if (!value) { return defaultValue; }
    std::string opt(value);
    if (opt.compare("true")==0) { return true; }
    return false;
  }
  inline unsigned long read_int(const char *optionName, unsigned long defaultValue = 0)
  {
    char *value = getenv(optionName);
    if (!value) { return defaultValue; }
    std::string opt(value);
#ifdef AC_READ_ENV_UNSAFE
    return atol(opt.c_str());
#else
    return std::stoul(opt);
#endif
  }
  inline float read_float(const char *optionName, float defaultValue = 0.0)
  {
    char *value = getenv(optionName);
    if (!value) { return defaultValue; }
    std::string opt(value);
#ifdef AC_READ_ENV_UNSAFE
    return atof(opt.c_str());
#else
    return std::stof(opt);
#endif
  }
};

#endif


/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 3.4                                                 *
 *                                                                        *
 *  Release Date    : Wed May  4 10:47:29 PDT 2022                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 3.4.3                                               *
 *                                                                        *
 *  Copyright 2018 Siemens                                                *
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
// =========================TESTBENCH=======================================
// This testbench file contains a stand-alone testbench that exercises the
// ac_arccos_cordic() function using a variety of bit-widths.

// To compile standalone and run:
//   $MGC_HOME/bin/c++ -std=c++11 -I$MGC_HOME/shared/include rtest_ac_arccos_cordic.cpp -o design
//   ./design

// Include the AC Math function that is exercised with this testbench
#include <ac_math/ac_arccos_cordic.h>
using namespace ac_math;

//==============================================================================
// Test Design
//   This simple function allows executing the ac_arccos_cordic() function.
//   Template parameters are used to configure the bit-widths of the types.

template <int Wfi, int Ifi, bool Sfi, int outWfi, int outIfi, bool outSfi>
void test_ac_arccos_cordic(
  const    ac_fixed<Wfi, Ifi, Sfi, AC_TRN, AC_WRAP>       &in,
  ac_fixed<outWfi, outIfi, outSfi, AC_TRN, AC_WRAP>  &out_cos
)
{
  ac_arccos_cordic(in, out_cos);
}

//==============================================================================

#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

//==============================================================================
// Function: test_driver()
// Description: A templatized function that can be configured for certain bit-
//   widths of the fixed point AC datatype. It uses the type information to
//   iterate through a range of valid values on that type in order to compare
//   the precision of the cordic based arccosine model with arccosine using
//   the standard C math library. The maximum error for each type is accumulated
//   in variables defined in the calling function.

template <int Wfi, int Ifi, int Sfi, int outWfi, int outIfi, bool outSfi>
int test_driver(
  double &cummulative_max_error_arccosine,
  const double allowed_error,
  bool details = false
)
{
  bool passed = true;
  double max_error_arccosine   = 0.0; // reset for this run

  ac_fixed<Wfi, Ifi, Sfi, AC_TRN, AC_WRAP>                 input;
  ac_fixed<outWfi, outIfi, outSfi, AC_TRN, AC_WRAP>      out_cos;

  typedef ac_fixed<outWfi, outIfi, outSfi, AC_TRN, AC_WRAP> T_out;

  double lower_limit, upper_limit, step;

  // set ranges and step size for the testbench
  lower_limit   = input.template set_val<AC_VAL_MIN>().to_double();
  upper_limit   = input.template set_val<AC_VAL_MAX>().to_double();
  step          = input.template set_val<AC_VAL_QUANTUM>().to_double();

  cout << "TEST: ac_arccos_cordic() INPUT: ";
  cout.width(38);
  cout << left << input.type_name();
  cout << "        OUTPUTS: ";
  cout.width(38);
  cout << left << out_cos.type_name();
  cout << "RESULT: ";

  // Dump the test details
  if (details) {
    cout << endl; // LCOV_EXCL_LINE
    cout << "  Ranges for input types:" << endl; // LCOV_EXCL_LINE
    cout << "    lower_limit    = " << lower_limit << endl; // LCOV_EXCL_LINE
    cout << "    upper_limit    = " << upper_limit << endl; // LCOV_EXCL_LINE
    cout << "    step           = " << step << endl; // LCOV_EXCL_LINE
  }

  for (double i = -1; i < 1; i += step) {
    // Set values for input.
    input = i;
    test_ac_arccos_cordic(input, out_cos);

    double expected_arccosine_value   = ((T_out)(acos(input.to_double())/M_PI)).to_double();
    double actual_arccosine_value       = out_cos.to_double();
    double this_error_arccosine;

    this_error_arccosine   = fabs(expected_arccosine_value - actual_arccosine_value) * 100.0;

    if (this_error_arccosine > max_error_arccosine) {max_error_arccosine = this_error_arccosine;}

  }

  if (passed) { printf("PASSED , max err (%f sin) \n", max_error_arccosine); }
  else        { printf("FAILED , max err (%f sin) \n", max_error_arccosine); } // LCOV_EXCL_LINE

  if (max_error_arccosine>cummulative_max_error_arccosine) { cummulative_max_error_arccosine = max_error_arccosine; }

  return 0;
}


int main(int argc, char *argv[])
{
  double max_error_arccosine = 0;
  double allowed_error = 0.1;
  cout << "=============================================================================" << endl;
  cout << "Testing function: ac_arccos_cordic() - Allowed error " << allowed_error << endl;

  // template <int Wfi, int Ifi, int Sfi>
  test_driver< 12,  1,  true, 24, 2, false>(max_error_arccosine, allowed_error);
  test_driver<  2,  0,  true, 27, 2, false>(max_error_arccosine, allowed_error);
  test_driver< 12,  0,  true, 24, 3, false>(max_error_arccosine, allowed_error);
  test_driver<  4,  1,  true, 25, 3, false>(max_error_arccosine, allowed_error);
  test_driver<  5, -2,  true, 30, 4, false>(max_error_arccosine, allowed_error);
  test_driver<  3, -2,  true, 28, 2, false>(max_error_arccosine, allowed_error);
  test_driver<  2, -2,  true, 32, 2, false>(max_error_arccosine, allowed_error);
  test_driver< 11,  1,  true, 34, 2, false>(max_error_arccosine, allowed_error);
  test_driver<  8, -3,  true, 25, 2, false>(max_error_arccosine, allowed_error);
  test_driver<  9, -3,  true, 26, 2, false>(max_error_arccosine, allowed_error);
  test_driver< 14,  2,  true, 24, 2, false>(max_error_arccosine, allowed_error);
  test_driver< 12,  2,  true, 24, 2, false>(max_error_arccosine, allowed_error);

  cout << "=============================================================================" << endl;
  cout << "  Testbench finished. Maximum errors observed across all bit-width variations:" << endl;
  cout << "    max_error_arccosine       = " << max_error_arccosine   << endl;

  // If error limits on any tested datatype have been crossed, the test has failed
  bool test_fail = max_error_arccosine > allowed_error;

  // Notify the user that the test was a failure.
  if (test_fail) {
    cout << "  ac_arccos_cordic - FAILED - Error tolerance(s) exceeded" << endl; // LCOV_EXCL_LINE
    cout << "=============================================================================" << endl; // LCOV_EXCL_LINE
    return -1; // LCOV_EXCL_LINE
  } else {
    cout << "  ac_arccos_cordic - PASSED" << endl;
    cout << "=============================================================================" << endl;
  }
  return 0;
}



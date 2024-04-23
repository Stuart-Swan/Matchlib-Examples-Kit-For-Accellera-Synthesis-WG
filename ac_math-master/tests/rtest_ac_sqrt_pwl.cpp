/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 3.5                                                 *
 *                                                                        *
 *  Release Date    : Thu Feb  8 17:36:42 PST 2024                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 3.5.0                                               *
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
// ac_sqrt_pwl() function using a variety of data types and bit-
// widths.

// To compile standalone and run:
//   $MGC_HOME/bin/c++ -std=c++11 -I$MGC_HOME/shared/include rtest_ac_sqrt_pwl.cpp -o design
//   ./design

// Include the AC Math function that is exercised with this testbench
#include <ac_math/ac_sqrt_pwl.h>
using namespace ac_math;

// ==============================================================================
// Test Design
//   These simple functions allow executing the ac_sqrt_pwl() function
//   using multiple data types at the same time. Template parameters are
//   used to configure the bit-widths of the types.

// Test Design for real and complex fixed point values.
template <int Wfi, int Ifi, int outWfi, int outIfi>
void test_ac_sqrt_pwl_fixed(
  const  ac_fixed<Wfi, Ifi, false, AC_TRN, AC_WRAP>   &in1,
  ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP>   &out1,
  const  ac_complex<ac_fixed<Wfi + 1, Ifi + 1, true, AC_TRN, AC_WRAP> > &in2,
  ac_complex<ac_fixed<outWfi + 1, outIfi + 1, true, AC_TRN, AC_WRAP> > &out2
)
{
  out1 = ac_sqrt_pwl<ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP> >(in1);
  out2 = ac_sqrt_pwl<ac_complex<ac_fixed<outWfi + 1, outIfi + 1, true, AC_TRN, AC_WRAP> > >(in2);
}

// Test Design for real fixed point values.
template <int Wfi, int Ifi, int outWfi, int outIfi>
void test_ac_sqrt_pwl_real_fixed(
  const  ac_fixed<Wfi, Ifi, false, AC_TRN, AC_WRAP>   &in1,
  ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP>   &out1
)
{
  out1 = ac_sqrt_pwl<ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP> >(in1);
}

// Test Design for real floating point values.
template <int Wfl, int Ifl, int Efl, int outWfl, int outIfl, int outEfl>
void test_ac_sqrt_pwl_float(
  const ac_float<Wfl, Ifl, Efl, AC_TRN>     &in1,
  ac_float<outWfl, outIfl, outEfl, AC_TRN>  &out1
)
{
  out1 = ac_sqrt_pwl<ac_float<outWfl, outIfl, outEfl, AC_TRN> >(in1);
}

// ==============================================================================

#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

// ------------------------------------------------------------------------------
// Helper functions for error calculation and monotonicity checks.

// Calculating error for real datatype.
template <class T_in, class T_out>
double err_calc(
  const T_in input,
  double &actual_value,
  const T_out output,
  const double allowed_error,
  const double threshold
)
{
  // The typecasting is done in order to provide quantization on the expected output.
  double expected_value = ((T_out)(sqrt(input.to_double()))).to_double();
  actual_value   = output.to_double();
  double this_error;

  // If expected value is greater than a particular threshold, calculate relative error, else, calculate absolute error.
  if (abs(expected_value) > threshold) {
    this_error = abs( (expected_value - actual_value) / expected_value ) * 100.0;
  } else {
    this_error = abs(expected_value - actual_value) * 100.0;
  }

#ifdef DEBUG
  if (this_error > allowed_error) {
    cout << endl;
    cout << "  Error tolerance exceeded for following values: " << endl;
    cout << "  input          = " << input << endl;
    cout << "  output         = " << output << endl;
    cout << "  expected_value = " << expected_value << endl;
    cout << "  this_error     = " << this_error << endl;
    assert(false);
  }
#endif

  return this_error;
}

template <class T_in, class T_out>
double cmplx_err_calc(
  const ac_complex<T_in> input,
  const ac_complex<T_out> output,
  const double allowed_error,
  const double threshold
)
{
  double in_r = input.r().to_double();
  double in_i = input.i().to_double();
  double mod = in_r*in_r + in_i*in_i;

  // Declare variables to store the expected output value, store the difference between
  // expected and actual output values and store the actual output value (converted to
  // double)
  ac_complex<double> exp_op, diff_op, act_op;

  // Convert actual output to double and store it in a separate complex variable.
  act_op.r() = output.r().to_double();
  act_op.i() = output.i().to_double();

  // Calculate the expected value using the C++ math library
  exp_op.r() = sqrt ((sqrt(mod) + in_r)/2);
  exp_op.i() = sqrt ((abs (-sqrt(mod) + in_r))/2);
  double y_neg = -exp_op.i();
  // handling sign of square root of complex number
  exp_op.i() = (in_i) < 0? y_neg : exp_op.i();

#ifdef DEBUG
  // Store the value before quantization. This can come in handy when debugging.
  ac_complex<double> exp_op_no_quant = exp_op;
#endif

  // Perform quantization on the expected output by converting it to the output of the expected
  // value, and then converting the quantized output back to a double.
  exp_op.r() = ((T_out)exp_op.r()).to_double();
  exp_op.i() = ((T_out)exp_op.i()).to_double();

  diff_op = exp_op - act_op;
  double error;

  if (sqrt(exp_op.mag_sqr()) != 0 && sqrt(exp_op.mag_sqr()) > threshold) {error = sqrt((diff_op / exp_op).mag_sqr()) * 100;}
  else {error = sqrt(diff_op.mag_sqr()) * 100;}

#ifdef DEBUG
  if (error > allowed_error) {
    cout << endl;
    cout << "  Error tolerance exceeded for following values: " << endl;
    cout << "  error           = " << error << endl;
    cout << "  input           = " << input << endl;
    cout << "  mod             = " << mod << endl;
    cout << "  exp_op_no_quant = " << exp_op_no_quant << endl;
    cout << "  exp_op          = " << exp_op << endl;
    cout << "  act_op          = " << act_op << endl;
    assert(false);
  }

  // If in case the calculations for the expected value are wrong, then the double output without
  // quantization is quite likely to have "nan" as the value for the real and/or imaginary part.
  // The assert below notifies the user of that.
  if (isnan(float(exp_op_no_quant.r())) || isnan(float(exp_op_no_quant.i()))) {
    cout << "  Real and/or imaginary parts of the calculated expected output were set to nan. Please check your calculations." << endl;
    assert(false);
  }
#endif

  return error;
}

// Function for monotonicity checking in ac_fixed inputs.
template <int Wfi, int Ifi, bool Sfi, int outWfi, int outIfi, bool outSfi>
void monotonicity_check(
  double &old_real_output,
  const double actual_value_fixed,
  bool &compare,
  ac_fixed<Wfi, Ifi, Sfi, AC_TRN, AC_WRAP> input_fixed,
  ac_fixed<outWfi, outIfi, outSfi, AC_TRN, AC_WRAP> output_fixed
)
{
  // MONOTONIC: Make sure that function is monotonic. Compare old value (value of previous iteration) with current value. Since the square root function we
  // are testing is an increasing function, and our testbench value keeps incrementing or remains the same (in case of saturation), we expect the
  // old value to be lesser than or equal to the current one.
  if (compare) {
    // Figuring out what the normalized value was for the input is a good way to figure out where the discontinuity occured w.r.t. the PWL segments.
    ac_fixed<Wfi, int(input_fixed.sign), input_fixed.sign, AC_TRN, AC_WRAP> norm_input_fixed;
    ac_normalize(input_fixed, norm_input_fixed);
    if (old_real_output > actual_value_fixed) {
      cout << endl; // LCOV_EXCL_LINE
      cout << "  Real, fixed point output not monotonic at :" << endl; // LCOV_EXCL_LINE
      cout << "  input_fixed = " << input_fixed << endl; // LCOV_EXCL_LINE
      cout << "  output_fixed = " << output_fixed << endl; // LCOV_EXCL_LINE
      cout << "  old_real_output = " << old_real_output << endl; // LCOV_EXCL_LINE
      cout << "  normalized x    = " << norm_input_fixed << endl; // LCOV_EXCL_LINE
      assert(false); // LCOV_EXCL_LINE
    }
  }
  // Update the variable for old_real_output.
  old_real_output = actual_value_fixed;
  // By setting compare to true, we make sure that once there is an old value stored, we can start comparing for monotonicity.
  compare = true;
}

// ==============================================================================
// Function: test_driver_fixed()
// Description: A templatized function that can be configured for certain bit-
//   widths of AC datatypes. It uses the type information to iterate through a
//   range of valid values on that type in order to compare the precision of the
//   piece-wise linear square root model with the computed square root using a
//   standard C double type. The maximum error for each type is accumulated
//   in variables defined in the calling function.
//   Number of iterations per run = 2^(2*Wfi).

template <int Wfi, int Ifi, int outWfi, int outIfi>
int test_driver_fixed(
  double &cumulative_max_error_fixed,
  double &cumulative_max_error_cmplx_fixed,
  const double allowed_error_fixed,
  const double allowed_error_complex,
  const double threshold,
  bool details = false
)
{
  bool passed = true;
  bool check_monotonic = true;
  double max_error_fixed = 0.0; // reset for this run
  double max_error_cmplx_fixed = 0.0; // reset for this run

  ac_fixed<   Wfi,    Ifi, false, AC_TRN, AC_WRAP>   input_fixed;
  ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP>   output_fixed;
  ac_complex<ac_fixed<   Wfi + 1,    Ifi + 1,  true, AC_TRN, AC_WRAP> > cmplx_input_fixed;
  ac_complex<ac_fixed<outWfi + 1, outIfi + 1,  true, AC_TRN, AC_WRAP> > cmplx_output_fixed;

  // set ranges and step size for fixed point testbench
  double step_fixed          = input_fixed.template set_val<AC_VAL_QUANTUM>().to_double();
  double lower_limit_complex = cmplx_input_fixed.r().template set_val<AC_VAL_MIN>().to_double();
  double upper_limit_complex = cmplx_input_fixed.r().template set_val<AC_VAL_MAX>().to_double();

  cout << "TEST: ac_sqrt_pwl() INPUTS: ";
  cout.width(38);
  cout << left << input_fixed.type_name();
  cout.width(50);
  cout << left << cmplx_input_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_fixed.type_name();
  cout.width(50);
  cout << left << cmplx_output_fixed.type_name();
  cout << "RESULT: ";

  // Dump the test details
  if (details) {
    cout << endl; // LCOV_EXCL_LINE
    cout << "  Ranges for input types:" << endl; // LCOV_EXCL_LINE
    cout << "    lower_limit_complex = " << lower_limit_complex << endl; // LCOV_EXCL_LINE
    cout << "    upper_limit_complex = " << upper_limit_complex << endl; // LCOV_EXCL_LINE
    cout << "    step_fixed          = " << step_fixed << endl; // LCOV_EXCL_LINE
  }

  double old_real_output;
  double actual_value_fixed;

  // test fixed-point real and complex.
  for (double i = lower_limit_complex; i <= upper_limit_complex; i += step_fixed) {
    bool compare = false;
    for (double j = lower_limit_complex; j <= upper_limit_complex; j += step_fixed) {
      cmplx_input_fixed.r() = i;
      cmplx_input_fixed.i() = j;
      input_fixed = j;
      // A negative imaginary value will correspond to an unwanted value for the real input. To
      // avoid this, set the real input to zero.
      if (cmplx_input_fixed.i() < 0) { input_fixed = 0; }
      test_ac_sqrt_pwl_fixed(input_fixed, output_fixed, cmplx_input_fixed, cmplx_output_fixed);

      double this_error_fixed = err_calc(input_fixed, actual_value_fixed, output_fixed, allowed_error_fixed, threshold);
      double this_error_complex = cmplx_err_calc(cmplx_input_fixed, cmplx_output_fixed, allowed_error_complex, threshold);

      if (check_monotonic) { monotonicity_check(old_real_output, actual_value_fixed, compare, input_fixed, output_fixed); }
      if (this_error_fixed > max_error_fixed) {max_error_fixed = this_error_fixed;}
      if (this_error_complex > max_error_cmplx_fixed) {max_error_cmplx_fixed = this_error_complex;}
    }
  }

  passed = (max_error_fixed < allowed_error_fixed) && (max_error_cmplx_fixed < allowed_error_complex);

  if (passed) { printf("PASSED , max error (%f) (%f complex)\n", max_error_fixed, max_error_cmplx_fixed); }
  else        { printf("FAILED , max error (%f) (%f complex)\n", max_error_fixed, max_error_cmplx_fixed); } // LCOV_EXCL_LINE

  if (max_error_fixed>cumulative_max_error_fixed) { cumulative_max_error_fixed = max_error_fixed; }
  if (max_error_cmplx_fixed>cumulative_max_error_cmplx_fixed) { cumulative_max_error_cmplx_fixed = max_error_cmplx_fixed; }

  return 0;
}

// =================================================================================
// Function: test_driver_real_fixed()
// Description: A specialized case of the above function which only tests real
//   ac_fixed values, allowing the user to test larger bitwidths than the one
//   which tests both real and complex values. Number of iterations per run = 2^Wfi.

template <int Wfi, int Ifi, int outWfi, int outIfi>
int test_driver_real_fixed(
  double &cumulative_max_error_fixed,
  const double allowed_error_fixed,
  const double threshold,
  bool details = false
)
{
  bool passed = true;
  bool check_monotonic = true;
  double max_error_fixed = 0.0; // reset for this run

  ac_fixed<   Wfi,    Ifi, false, AC_TRN, AC_WRAP> input_fixed;
  ac_fixed<outWfi, outIfi, false, AC_TRN, AC_WRAP> output_fixed;

  // set ranges and step size for fixed point testbench
  double step_fixed        = input_fixed.template set_val<AC_VAL_QUANTUM>().to_double();
  double lower_limit_fixed = input_fixed.template set_val<AC_VAL_MIN>().to_double();
  double upper_limit_fixed = input_fixed.template set_val<AC_VAL_MAX>().to_double();

  cout << "TEST: ac_sqrt_pwl() INPUTS: ";
  cout.width(38);
  cout << left << input_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_fixed.type_name();
  cout << "RESULT: ";

  // Dump the test details
  if (details) {
    cout << endl; // LCOV_EXCL_LINE
    cout << "  Ranges for input types:" << endl; // LCOV_EXCL_LINE
    cout << "    lower_limit_fixed = " << lower_limit_fixed << endl; // LCOV_EXCL_LINE
    cout << "    upper_limit_fixed = " << upper_limit_fixed << endl; // LCOV_EXCL_LINE
    cout << "    step_fixed        = " << step_fixed << endl; // LCOV_EXCL_LINE
  }

  double old_real_output;
  double actual_value_fixed;

  bool compare = false;
  // Fix the real part of the input at 0, and iterate through all possible values of imaginary part based on its type.
  for (double i = lower_limit_fixed; i <= upper_limit_fixed; i += step_fixed) {
    input_fixed = i;

    // Pass all inputs at one go
    test_ac_sqrt_pwl_real_fixed(input_fixed, output_fixed);

    double this_error_fixed = err_calc(input_fixed, actual_value_fixed, output_fixed, allowed_error_fixed, threshold);

    if (check_monotonic) { monotonicity_check(old_real_output, actual_value_fixed, compare, input_fixed, output_fixed); }
    if (this_error_fixed > max_error_fixed) {max_error_fixed = this_error_fixed;}
  }

  passed = (max_error_fixed < allowed_error_fixed);

  if (passed) { printf("PASSED , max error (%f)\n", max_error_fixed); }
  else        { printf("FAILED , max error (%f)\n", max_error_fixed); } // LCOV_EXCL_LINE

  if (max_error_fixed>cumulative_max_error_fixed) { cumulative_max_error_fixed = max_error_fixed; }

  return 0;
}

// ==============================================================================
// Function: test_driver_float()
// Description: A templatized function that can be configured for certain bit-
//   widths of real ac_float datatypes. It uses the type information to iterate
//   through a range of valid values on that type in order to compare the
//   precision of the piece-wise linear sqrt model with the computed sqrt using a
//   standard C double type. The maximum error for each type is accumulated
//   in variables defined in the calling function.

template <int Wfl, int Ifl, int Efl, int outWfl, int outIfl, int outEfl>
int test_driver_float(
  double &cumulative_max_error_float,
  const double allowed_error_float,
  const double threshold,
  bool details = false
)
{
  bool passed = true;
  double max_error_float = 0.0; // reset for this run

  typedef ac_float<Wfl, Ifl, Efl, AC_TRN> T_in;
  typedef ac_float<outWfl, outIfl, outEfl, AC_TRN> T_out;

  // Since ac_float values are normalized, the bit adjacent to the sign bit in the mantissa
  // will always be set to 1. We will hence cycle through all the bit patterns that correspond to the last (Wfl - 2)
  // bits in the mantissa.
  ac_int<Wfl - 2, false> sample_mantissa_slc;
  // Set the lower limit, upper limit and step size of the test iterations.
  ac_int<Wfl - 2, false> lower_limit_it = 0;
  ac_int<Wfl - 2, false> upper_limit_it = sample_mantissa_slc.template set_val<AC_VAL_MAX>().to_double();
  ac_int<Wfl - 2, false> step_it = 1; // Since sample_mantissa_slc is an integer.

  string empty_str = "";

  cout << "TEST: ac_sqrt_pwl() INPUT:  ";
  cout.width(38);
  cout << left << T_in::type_name();
  cout.width(50);
  cout << left << empty_str;
  cout << "OUTPUT:  ";
  cout.width(38);
  cout << left << T_out::type_name();
  cout << "RESULT: ";

  // Declare arrays to store all values of exponent to be tested.
  const int exp_arr_size = 2*(Efl - 1) + 3;
  ac_int<Efl, true> sample_exponent;
  ac_int<Efl, true> sample_exponent_array[exp_arr_size];

  // The first element of the array is the minimum exponent value, the middle element is a zero exponent, and
  // the last element is the maximum possible value.
  sample_exponent_array[0].template set_val<AC_VAL_MIN>();
  sample_exponent_array[Efl] = 0;
  sample_exponent_array[exp_arr_size - 1].template set_val<AC_VAL_MAX>();

  // All the other elements are set to values that correspond to a one-hot encoding scheme, in which only one
  // bit of the absolute value of the exponent is set to one. Both negative and positive values are encoded this way.
  for (int i = (Efl - 2); i >= 0; i--) {
    sample_exponent = 0;
    sample_exponent[i] = 1;
    sample_exponent_array[Efl + i + 1] = sample_exponent;
    sample_exponent_array[Efl - i - 1] = -sample_exponent;
  }

  // Dump the test details
  if (details) {
    cout << endl << "  Ranges for testing iterations:" << endl; // LCOV_EXCL_LINE
    cout         << "    lower_limit_it       = " << lower_limit_it << endl; // LCOV_EXCL_LINE
    cout         << "    upper_limit_it       = " << upper_limit_it << endl; // LCOV_EXCL_LINE
    cout         << "    step_it              = " << step_it << endl; // LCOV_EXCL_LINE
    cout         << "    allowed_error_float  = " << allowed_error_float << endl; // LCOV_EXCL_LINE
  }

  for (int i = 0; i < exp_arr_size; i++) {
    // For a particular exponent value, go through every possible value that can be represented by the mantissa.
    // The iteration variable has a bitwidth that is 1 higher (bitwidth = Wfl - 1) than the slice of the mantissa
    // we'll be changing from one iteration to the other (bitwidth of slice = Wfl - 2), to ensure that the loop variable
    // doesn't overflow.
    for (ac_int<Wfl - 1, false> mant_i = lower_limit_it; mant_i <= upper_limit_it; mant_i += step_it) {
      ac_fixed<Wfl, Ifl, true> input_mant;
      input_mant[Wfl - 1] = 0;
      // Set the bit adjacent to the sign bit to 1 to ensure a normalized mantissa
      input_mant[Wfl - 2] = 1;
      // Set the remaining bits to the bit pattern stored in the last (Wfl - 2) bits in mant_i.
      input_mant.set_slc(0, mant_i.template slc<Wfl - 2>(0));
      // Use a parameterized ac_float constructor to set the mantissa and exponent of the temporary floating point input.
      T_in input_float(input_mant, sample_exponent_array[i], true);
      // Make sure that input_mant was normalized and that the mantissa and exponent values haven't changed after calling the constructor.
      if (input_float.mantissa() != input_mant || input_float.exp() != sample_exponent_array[i]) {
        cout << "input_mant was not normalized correctly." << endl;
        assert(false);
      }
      T_out output_float;
      test_ac_sqrt_pwl_float(input_float, output_float);
      double actual_value_float = output_float.to_double();
      double this_error_float = err_calc(input_float, actual_value_float, output_float, allowed_error_float, threshold);

      if (this_error_float > max_error_float) {max_error_float = this_error_float;}
    }
  }
  passed = (max_error_float < allowed_error_float);

  if (passed) { printf("PASSED , max error (%f) \n", max_error_float); }
  else        { printf("FAILED , max error (%f) \n", max_error_float); } // LCOV_EXCL_LINE

  if (max_error_float > cumulative_max_error_float) { cumulative_max_error_float = max_error_float; }

  return 0;
}


int main(int argc, char *argv[])
{
  double max_error_fixed = 0, cmplx_max_error_fixed = 0, max_error_float = 0;
  double allowed_error_fixed = 0.5;
  // ac_complex values for input and output often give a higher error than ac_fixed values,
  // owing to certain intermediate computations in the sqrt function.
  // Hence, it is best to define a separate error tolerance for them.
  double allowed_error_complex = 3;
  double allowed_error_float = 0.5;
  // threshold below which we calculate absolute error instead of relative

  const double threshold = 0.005;
  cout << "=============================================================================" << endl;
  cout << "Testing function: ac_sqrt_pwl() - Allowed error " << allowed_error_fixed << " (fixed pt), " << allowed_error_complex << " (complex), " << allowed_error_float << " (float pt)" << endl;

  // template <int Wfi, int Ifi, int outWfi, int outIfi>
  test_driver_fixed<  8,-12, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed< 11,  0, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  4,  9, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  4, -2, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  5,  8, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  4, -2, 60, 30>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed< 10,  4, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed< 10,  3, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  9,  4, 60, 30>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);
  test_driver_fixed<  9,  2, 64, 32>(max_error_fixed, cmplx_max_error_fixed, allowed_error_fixed, allowed_error_complex, threshold);

  // template <int Wfi, int Ifi, int outWfi, int outIfi>
  test_driver_real_fixed< 21,  8, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  7, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  6, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  3, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  2, 60, 30>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  1, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21,  0, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21, -1, 60, 30>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21, -2, 63, 33>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 21, -3, 60, 30>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 11, 20, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 11, 21, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);
  test_driver_real_fixed< 11, 22, 64, 32>(max_error_fixed, allowed_error_fixed, threshold);

  // template <int Wfl, int Ifl, int Efl, int outWfl, int outIfl, int outEfl>
  test_driver_float< 21,  9, 8, 21,  9,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21,  8, 8, 21,  8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21,  0, 8, 21,  0,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 20,  0, 8, 20,  8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21,  1, 8, 21,  1,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21,  2, 8, 21,  2,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21,  3, 8, 21,  3,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19,  1, 8, 18,  8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19,  2, 8, 18,  8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19,  3, 8, 18,  8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21, -1, 8, 21, -1,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21, -2, 8, 21, -2,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21, -3, 8, 21, -3,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 21, -3, 8, 21, -3,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19, -1, 8, 18, -8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19, -2, 8, 18, -8,  8>(max_error_float, allowed_error_float, threshold);
  test_driver_float< 19, -3, 8, 18, -8,  8>(max_error_float, allowed_error_float, threshold);

  cout << "=============================================================================" << endl;
  cout << "  Testbench finished. Maximum errors observed across all data type / bit-width variations:" << endl;
  cout << "    max_error_fixed       = " << max_error_fixed << endl;
  cout << "    cmplx_max_error_fixed = " << cmplx_max_error_fixed << endl;
  cout << "    max_error_float       = " << max_error_float << endl;

  // If error limits on any tested datatype have been crossed, the test has failed
  bool test_fail = (max_error_fixed > allowed_error_fixed) || (cmplx_max_error_fixed > allowed_error_complex) || (max_error_float > allowed_error_float);

  // Notify the user that the test was a failure.
  if (test_fail) {
    cout << "  ac_sqrt_pwl - FAILED - Error tolerance(s) exceeded" << endl; // LCOV_EXCL_LINE
    cout << "=============================================================================" << endl; // LCOV_EXCL_LINE
    return -1; // LCOV_EXCL_LINE
  } else {
    cout << "  ac_sqrt_pwl - PASSED" << endl;
    cout << "=============================================================================" << endl;
  }
  return 0;
}



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
// Usage:
//   g++ -I$MGC_HOME/shared/include ac_sincos_lut_lutgen.cpp -o ac_sincos_lut_lutgen
//   ./ac_sincos_lut_lutgen
// results in a text file ac_sincos_lut_values.txt which can be pasted into
// a locally modified version of ac_sincos_lut.h.

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<stdio.h>
#include<math.h>

//==========================================================================================
// Note:
//      This file is used to generate a lookup table for the ac_sincos_lut.h library header.
//      In this file, a lookup table has been generated for an input width of 12 bits and
//      input integer width of 0 bits. So for example, if a lookup table has to be generated
//      for an input width of 14 bits and input integer width of 2 bits, then the variables
//      input_width and input_int have to be replaced accordingly.
//------------------------------------------------------------------------------------------

int main()
{

  FILE *f = fopen("ac_sincos_lut.txt", "w");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }

  const int input_width = 12;
  const int input_int = 0;

  unsigned int NTE = 1<<(input_width - input_int -3); //No of table entries
  double step = M_PI/(4*NTE);                         //Interval between angles
  double y = 0;
  double scaled_angle = 0;

  fprintf(f, "static const luttype sincos[%d] = { \n", NTE);

  for (unsigned int i=0; i < NTE; i++) {
    fprintf(f, "  {%23.22f, %23.22f}, //index = %d, scaled angle = %13.12f \n", cos(y), sin(y), i, scaled_angle);
    y += step;
    scaled_angle = y/(2*M_PI);
  }

  fclose(f);

  return 0;

}

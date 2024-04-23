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
//*****************************************************************************************************
// File: ac_sincos_cordic.h
//
// Description:
//    Provides cordic based implementation of the sine and cosine functions for the
//    AC Datatype ac_fixed.
//
// Usage:
//    A sample testbench and its implementation look like
//    this:
//
//    #include <ac_fixed.h>
//    #include <ac_math/ac_sincos_cordic.h>
//    using namespace ac_math;
//
//    typedef ac_fixed<20, 11, true, AC_RND, AC_SAT> input_type;
//    typedef ac_fixed<24, 14, true, AC_RND, AC_SAT> output_type;
//
//    #pragma hls_design top
//    void project(
//      const input_type &input,
//      output_type &output,
//    )
//    {
//      ac_sin_cordic(input, sin_output);
//      ac_cos_cordic(input, cos_output);
//    }
//
//    #ifndef __SYNTHESIS__
//    #include <mc_scverify.h>
//
//    CCS_MAIN(int arg, char **argc)
//    {
//      input_type input = 1.5;
//      output_type sin_output;
//      output_type cos_output;
//      CCS_DESIGN(project)(input, sin_output, cos_output);
//      CCS_RETURN (0);
//    }
//    #endif
//
// Revision History:
//    3.3.0  - [CAT-25798] Added CDesignChecker fixes/waivers for code check and Synthesis-simulation mismatch/violations in ac_math PWL and Linear Algebra IPs.
//    3.1.2  - Renamed typedef to avoid redeclaration conflict with typedef in ac_atan2_cordic.h
//    2.0.10 - Official open-source release as part of the ac_math library.
//
//*****************************************************************************************************

#ifndef _INCLUDED_AC_SINCOS_CORDIC_H_
#define _INCLUDED_AC_SINCOS_CORDIC_H_

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

#include <ac_fixed.h>
#include <ac_float.h>
#include <ac_std_float.h>
#include <ac_math/ac_shift.h>

// The computation of the K table using double arithmetic
//  limits what practical TE could be chosen.

#define TE 70

#define M_PI            3.14159265358979323846

namespace ac_math
{

  typedef ac_fixed<TE+2,1,true,AC_RND,AC_WRAP> table_st;

  // Scaled atan
  static table_st atan_pi_pow2_table[] = {
    .25,
    .1475836176504332630798899117507971823215484619140625,
    .07797913037736932395649347427024622447788715362548828125,
    .039583424160565545968371026219756458885967731475830078125,
    .0198685243055408411461115747442818246781826019287109375,
    .00994394782358927475929277761679259128868579864501953125,
    .00497318727895041355624439205485032289288938045501708984375,
    .00248674539366973944287675379882784909568727016448974609375,
    .001243391668714100527226573689176802872680127620697021484375,
    .000621698205923371625958306907477890490554273128509521484375,
    .0003108493994100203889306699434058600672869943082332611083984375,
    .00015542473676113154946488992269593154560425318777561187744140625,
    .000077712373012583417443595312779081041298923082649707794189453125,
    .0000388561870852939953758543867312624797705211676657199859619140625,
    .00001942809361502228351968428465834648477539303712546825408935546875,
    .000009714046816558053335844725995062987067285575903952121734619140625,
    .0000048570234094098901914062123286175420844301697798073291778564453125,
    .00000242851170484630303613858733069452711106350761838257312774658203125,
    .0000012142558524408212606237288111454830641378066502511501312255859375,
    .0000006071279262226192687218299937068621829894254915416240692138671875,
    .00000030356396311158576710171989876396736462993430905044078826904296875,
    .00000015178198155582740014346056212080071645686984993517398834228515625,
    .00000007589099077791814699470336613718046692156349308788776397705078125,
    .0000000379454953889597087720621237938445347026572562754154205322265625,
    .00000001897274769447985438603106189692226735132862813770771026611328125,
    .00000000948637384724013895125234452354590075628948397934436798095703125,
    .000000004743186923619857717389358686688183297519572079181671142578125,
    .0000000023715934618099288586946793433440916487597860395908355712890625,
    .00000000118579673090496442934733967167204582437989301979541778564453125,
    .00000000059289836545269397291048341092078999281511642038822174072265625,
    .0000000002964491827261352282184281303756279157823882997035980224609375,
    .00000000014822459136306761410921406518781395789119414985179901123046875,
    .000000000074112295681745565291420607678674059570766985416412353515625,
    .0000000000370561478408727826457103038393370297853834927082061767578125,
    .00000000001852807392043639132285515191966851489269174635410308837890625,
    .0000000000092640369600064374246140008750671768211759626865386962890625,
    .00000000000463201848000321871230700043753358841058798134326934814453125,
    .00000000000231600924021336759296707530353387483046390116214752197265625,
    .0000000000011580046198949255596699625669998567900620400905609130859375,
    .00000000000057900230994746277983498128349992839503102004528045654296875,
    .000000000000289501155185489626731065726517044822685420513153076171875,
    .0000000000001447505775927448133655328632585224113427102565765380859375,
    .00000000000007237528879637240668276643162926120567135512828826904296875,
    .00000000000003618764418642796652780813104754997766576707363128662109375,
    .0000000000000180938223049722200774791502908556140027940273284912109375,
    .00000000000000904691115248611003873957514542780700139701366424560546875,
    .00000000000000452345536448481820579470280563327833078801631927490234375,
    .0000000000000022617278940006459164724361698972643353044986724853515625,
    .00000000000000113086394700032295823621808494863216765224933624267578125,
    .0000000000000005654317617419246655430242753936909139156341552734375,
    .00000000000000028271588087096233277151213769684545695781707763671875,
    .000000000000000141357940435481166385756068848422728478908538818359375,
    .0000000000000000706789702177405831928780344242113642394542694091796875,
    .00000000000000003533948510887029159643901721210568211972713470458984375,
    .0000000000000000176699543126719593733042756866780109703540802001953125,
    .00000000000000000883497715633597968665213784333900548517704010009765625,
    .0000000000000000044172768199311762682413018410443328320980072021484375,
    .00000000000000000220863840996558813412065092052216641604900360107421875,
    .0000000000000000011045309632196076421450925408862531185150146484375,
    .00000000000000000055226548160980382107254627044312655925750732421875,
    .000000000000000000276132740804901910536273135221563279628753662109375,
    .0000000000000000001380663704024509552681365676107816398143768310546875,
    .00000000000000000006903318520122547763406828380539081990718841552734375,
    .00000000000000000003430483436379916373226706127752549946308135986328125,
    .00000000000000000001736417541871315695090061126393266022205352783203125,
    .000000000000000000008470329472543003390683225006796419620513916015625,
    .0000000000000000000042351647362715016953416125033982098102569580078125,
    .00000000000000000000211758236813575084767080625169910490512847900390625,
    .00000000000000000000127054942088145050860248375101946294307708740234375,
    .00000000000000000000042351647362715016953416125033982098102569580078125
  };

  static table_st K_table[] = {
    -1.0,
      .707106781186547461715008466853760182857513427734375,
      .63245553203367588235295215781661681830883026123046875,
      .6135719910778962837838435007142834365367889404296875,
      .6088339125177524291387953780940733850002288818359375,
      .60764825625616813997709186878637410700321197509765625,
      .60735177014129593242586224732804112136363983154296875,
      .60727764409352602559266642856528051197528839111328125,
      .60725911229889273368343083348008804023265838623046875,
      .60725447933256238020049977421876974403858184814453125,
      .607253321089875175431416209903545677661895751953125,
      .6072530315291342351002867872011847794055938720703125,
      .6072529591389448366811620871885679662227630615234375,
      .60725294104139704298717106212279759347438812255859375,
      .60725293651701017783040015274309553205966949462890625,
      .6072529353859135170523586566559970378875732421875,
      .607252935103139268591121435747481882572174072265625,
      .60725293503244570647581213052035309374332427978515625,
      .6072529350147723992137116511003114283084869384765625,
      .607252935010353933620308453100733458995819091796875,
      .60725293500924937273310888485866598784923553466796875,
      .60725293500897326026688460842706263065338134765625,
      .60725293500890431541705538620590232312679290771484375,
      .60725293500888699593787123376387171447277069091796875,
      .60725293500888266606807519565336406230926513671875,
      .6072529350088815558450505704968236386775970458984375,
      .607252935008881333800445645465515553951263427734375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375,
      .60725293500888122277814318294986151158809661865234375
    };

  static table_st atan_pi_2mi(int i)
  {
    #pragma hls_waive CNS
    if (i >= TE)
    { return 0.0; }
    return atan_pi_pow2_table[i];
  }

  static table_st K(int n)
  {
    #pragma hls_waive CNS
    if (n >= TE)
    { return 0.0; }
    return K_table[n];
  }

  //============================================================================
  // Function:  C*sin(a*PI), C*cos(a*PI) (for ac_fixed)
  //
  // Description:
  //     -  Inputs:
  //          - angle scaled by PI,
  //          - scaling factor C
  //     -  Outputs:
  //          - sin
  //          - cos
  //
  //     Angle is expected as radians scaled by 1/PI
  //     This assumption makes it easy to determine what quadrant
  //     the angle is in. Also it saves a multiplication in
  //     a typical call.  For instance instead of 2*PI*i/n,
  //     we call it with 2*i/n.
  //-------------------------------------------------------------------------

  template< int AW, int AI, ac_q_mode AQ, ac_o_mode AO,
            int OW, int OI, ac_q_mode OQ, ac_o_mode OO >
  void ac_sincos_cordic(
    ac_fixed<AW,AI,true,AQ,AO> angle_over_pi,
    ac_fixed<OW,OI,true,OQ,OO> C,
    ac_fixed<OW,OI,true,OQ,OO> &sin,
    ac_fixed<OW,OI,true,OQ,OO> &cos
  )
  {
    // Number of iterations depends on output precision OW
    const int N_I  = (OW+1);

    // assume maximal N_I of 127
    const int LOG_N_I = (N_I < 16) ? 4 : (N_I < 32) ? 5 : (N_I < 64) ? 6 : 7;
    // Precision for internal computation: n + log(n)
    const int ICW = (N_I + LOG_N_I);

    typedef ac_fixed<ICW,OI,true> fx_xy;

    fx_xy x, y;

    // Accumulator may actually swing slightly out of target range
    //  -1 to 1  (-PI to PI). So make the range -2 to 2. Thus 2
    //  bits for integer part
    // Precision for angle accumulator: n + log(n) + 2
    ac_fixed<ICW+2,2,true> acc_a;

    // throw away upper bits to make target_a in correct range:
    //    -1 <= target_a < 1   (-PI <= angle < PI)
    ac_fixed<AW-(AI-1),1,true> target_a = angle_over_pi;

    fx_xy k = K(N_I);
    fx_xy K_x_scale = C * k;

    if (target_a > (ac_fixed<1,0,false>) 0.5) {  // PI/2 <= angle < PI/2
      x = 0.0;
      y = K_x_scale;
      acc_a = 0.5;
    } else if (target_a < (ac_fixed<1,0,true>) -0.5) { // -PI <= angle < -PI/2
      x = 0.0;
      y = -K_x_scale;
      acc_a = -0.5;
    } else { // -PI/2 <= angle <= PI/2
      x = K_x_scale;
      y = 0.0;
      acc_a = 0.0;
    }

    for (int i = 0; i < N_I; i++) {
      ac_fixed<ICW+1,1,true> d_a = atan_pi_2mi(i);
      fx_xy x_2mi = x >> i;  // x * pow(2, -i)
      fx_xy y_2mi = y >> i;  // y * pow(2, -i)
      if (target_a < acc_a) {
        x += y_2mi;
        y -= x_2mi;
        acc_a -= d_a;
      } else {
        x -= y_2mi;
        y += x_2mi;
        acc_a += d_a;
      }
    }
    sin = y;
    cos = x;
  }

  template< int AW, int AI, ac_q_mode AQ, ac_o_mode AO,
            int OW, int OI, ac_q_mode OQ, ac_o_mode OO >
  void ac_sin_cordic(
    const ac_fixed<AW,AI,true,AQ,AO> angle_over_pi,
    ac_fixed<OW,OI,true,OQ,OO> &sin
  )
  {
    ac_fixed<OW,OI,true,OQ,OO> scale = 1.0;
    ac_fixed<OW,OI,true,OQ,OO> cos;
    ac_sincos_cordic(angle_over_pi, scale, sin, cos);
  }

  template< int AW, int AI, ac_q_mode AQ, ac_o_mode AO,
            int OW, int OI, ac_q_mode OQ, ac_o_mode OO >
  void ac_cos_cordic(
    const ac_fixed<AW,AI,true,AQ,AO> angle_over_pi,
    ac_fixed<OW,OI,true,OQ,OO> &cos
  )
  {
    ac_fixed<OW,OI,true,OQ,OO> scale = 1.0;
    ac_fixed<OW,OI,true,OQ,OO> sin;
    ac_sincos_cordic(angle_over_pi, scale, sin, cos);
  }

  //============================================================================
  // Function:  C*sin(a*PI), C*cos(a*PI) (for ac_float)
  //
  // Description:
  //     -  Inputs:
  //          - angle scaled by PI,
  //          - scaling factor C
  //     -  Outputs:
  //          - sin
  //          - cos
  //
  //     Angle is expected as radians scaled by 1/PI
  //     This assumption makes it easy to determine what quadrant
  //     the angle is in. Also it saves a multiplication in
  //     a typical call.  For instance instead of 2*PI*i/n,
  //     we call it with 2*i/n.
  //-------------------------------------------------------------------------

  template< int AW, int AI, int AE, ac_q_mode AQ,
            int OW, int OI, int OE, ac_q_mode OQ >
  void ac_sin_cordic(
    const ac_float<AW,AI,AE,AQ> &angle_over_pi,
    ac_float<OW,OI,OE,OQ> &sin
  )
  {
    ac_fixed<AW, AI, true> mantVal = angle_over_pi.mantissa();
    int exp_val = angle_over_pi.exp().to_int();
    // Intermediate ac_fixed variables to store the value of inputs and outputs
    // and enable compatibility with ac_fixed implementation.
    ac_fixed<40, 15, true, AC_RND> angle_input_fi;
    // Use ac_shift_left instead of "<<" operator to ensure rounding.
    ac_math::ac_shift_left(mantVal, exp_val, angle_input_fi);
    ac_fixed<35, 7, true> sin_output_fi;
    ac_sin_cordic(angle_input_fi, sin_output_fi);
    // Convert ac_fixed output to ac_float by using a constructor.
    ac_float<OW,OI,OE,OQ> output_temp(sin_output_fi);

    sin = output_temp;
  }

  template< int AW, int AI, int AE, ac_q_mode AQ,
            int OW, int OI, int OE, ac_q_mode OQ >
  void ac_cos_cordic(
    const ac_float<AW,AI,AE,AQ> &angle_over_pi,
    ac_float<OW,OI,OE,OQ> &cos
  )
  {
    // Input is always assumed to be positive -> sign bit is unnecessary.
    ac_fixed<AW, AI, true> mantVal = angle_over_pi.mantissa();
    int exp_val = angle_over_pi.exp().to_int();
    // Intermediate ac_fixed variables to store the value of inputs and outputs
    // and enable compatibility with ac_fixed implementation.
    ac_fixed<40, 15, true, AC_RND> angle_input_fi;
    // Use ac_shift_left instead of "<<" operator to ensure rounding.
    ac_math::ac_shift_left(mantVal, exp_val, angle_input_fi);
    ac_fixed<35, 7, true> cos_output_fi;
    ac_cos_cordic(angle_input_fi, cos_output_fi);
    // Convert ac_fixed output to ac_float by using a constructor.
    ac_float<OW,OI,OE,OQ> output_temp(cos_output_fi);

    cos = output_temp;
  }


//=========================================================================
// Function: ac_sin_cordic (for ac_std_float)
//
//-------------------------------------------------------------------------

  template <int W, int E, int outW, int outE>
  void ac_sin_cordic(
    const ac_std_float<W, E> &angle_over_pi,
    ac_std_float<outW, outE> &sin
  )
  {
    ac_float<outW - outE + 1, 2, outE> sin_ac_fl; // Equivalent ac_float representation for output.
    ac_sin_cordic(angle_over_pi.to_ac_float(), sin_ac_fl); // Call ac_float version.
    ac_std_float<outW, outE> output_temp(sin_ac_fl); // Convert output ac_float to ac_std_float.
    sin = output_temp;
  }

//=========================================================================
// Function: ac_cos_cordic (for ac_std_float)
//
//-------------------------------------------------------------------------

  template <int W, int E, int outW, int outE>
  void ac_cos_cordic(
    const ac_std_float<W, E> &angle_over_pi,
    ac_std_float<outW, outE> &cos
  )
  {
    ac_float<outW - outE + 1, 2, outE> cos_ac_fl; // Equivalent ac_float representation for output.
    ac_cos_cordic(angle_over_pi.to_ac_float(), cos_ac_fl); // Call ac_float version.
    ac_std_float<outW, outE> output_temp(cos_ac_fl); // Convert output ac_float to ac_std_float.
    cos = output_temp;
  }

//=========================================================================
// Function: ac_sin_cordic (for ac_ieee_float)
//
//-------------------------------------------------------------------------

  template<ac_ieee_float_format Format,
           ac_ieee_float_format outFormat>
  void ac_sin_cordic(
    const ac_ieee_float<Format> &angle_over_pi,
    ac_ieee_float<outFormat> &sin
  )
  {
    typedef ac_ieee_float<outFormat> T_out;
    const int outW = T_out::width;
    const int outE = T_out::e_width;
    ac_float<outW - outE + 1, 2, outE> sin_ac_fl; // Equivalent ac_float representation for output.
    ac_sin_cordic(angle_over_pi.to_ac_float(), sin_ac_fl); // Call ac_float version.
    ac_ieee_float<outFormat> output_temp(sin_ac_fl); // Convert output ac_float to ac_ieee_float.
    sin = output_temp;
  }

//=========================================================================
// Function: ac_cos_cordic (for ac_ieee_float)
//
//-------------------------------------------------------------------------

  template<ac_ieee_float_format Format,
           ac_ieee_float_format outFormat>
  void ac_cos_cordic(
    const ac_ieee_float<Format> &angle_over_pi,
    ac_ieee_float<outFormat> &cos
  )
  {
    typedef ac_ieee_float<outFormat> T_out;
    const int outW = T_out::width;
    const int outE = T_out::e_width;
    ac_float<outW - outE + 1, 2, outE> cos_ac_fl; // Equivalent ac_float representation for output.
    ac_cos_cordic(angle_over_pi.to_ac_float(), cos_ac_fl); // Call ac_float version.
    ac_ieee_float<outFormat> output_temp(cos_ac_fl); // Convert output ac_float to ac_ieee_float.
    cos = output_temp;
  }

} // namespace ac_math

#endif

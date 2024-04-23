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
//*************************************************************************************************************************************
// File: ac_arcsin_cordic.h
//
// Description:
//    Provides cordic based implementation of the inverse sine function for the
//    AC Datatype ac_fixed.
//
// Usage:
//    A sample testbench and its implementation look like
//    this:
//
//    #include <ac_fixed.h>
//    #include <ac_math/ac_arcsin_cordic.h>
//    using namespace ac_math;
//
//    typedef ac_fixed< 5,  3, true, AC_RND, AC_SAT> input_type;
//    typedef ac_fixed<12,  5, true, AC_RND, AC_SAT> output_type;
//
//    #pragma hls_design top
//    void project(
//      const input_type &input,
//      output_type &arcsin_output,
//    )
//    {
//      ac_arcsin_cordic(input, arcsin_output);
//    }
//
//    #ifndef __SYNTHESIS__
//    #include <mc_scverify.h>
//
//    CCS_MAIN(int arg, char **argc)
//    {
//      input_type input = 0.5;
//      output_type arcsin_output;
//      CCS_DESIGN(project)(input, arcsin_output);
//      CCS_RETURN (0);
//    }
//    #endif
//
// Revision History:
//    3.3.0  - [CAT-25798] Added CDesignChecker fixes/waivers for code check and Synthesis-simulation mismatch/violations in ac_math PWL and Linear Algebra IPs.
//    3.1.0  - Used names for the class, function and variable (which were same before) which are different from ac_arccos_cordic.h
//    2.0.10 - Official open-source release as part of the ac_math library.
//
//************************************************************************************************************************************

#ifndef _INCLUDED_AC_ARCSIN_CORDIC_H_
#define _INCLUDED_AC_ARCSIN_CORDIC_H_

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

  template <bool b> struct MgcAcIarcsinAssert {};
  template <> struct MgcAcIarcsinAssert<true> { static void test() {} };

  // Multi-precision approximation of 2*tan(2^-i)/pi
  typedef ac_fixed<128,0,false> val_t;
  static const ac_fixed<128,0,false> x2_asin_pi_pow2_table[] = {
    val_t( 5.0000000000000000e-001 ) + val_t( 0.0000000000000000e+000 ) + val_t( 0.0000000000000000e+000 ),
    val_t( 2.9516723530086653e-001 ) + val_t( 2.2191022328947885e-017 ) + val_t( 1.2208189343972259e-033 ),
    val_t( 1.5595826075473865e-001 ) + val_t( 3.0080388310057675e-018 ) + val_t( 2.9135660047960282e-034 ),
    val_t( 7.9166848321131078e-002 ) + val_t( 5.9627490980550198e-018 ) + val_t( 1.2463059795280529e-034 ),
    val_t( 3.9737048611081675e-002 ) + val_t( 2.7653905201348738e-018 ) + val_t( 8.9489733610604287e-035 ),
    val_t( 1.9887895647178546e-002 ) + val_t( 1.8080863942320186e-018 ) + val_t( 3.5878717221037795e-034 ),
    val_t( 9.9463745579008254e-003 ) + val_t( 3.2935932323465678e-019 ) + val_t( 3.0950261092971041e-035 ),
    val_t( 4.9734907873394780e-003 ) + val_t( 5.7154549511767913e-019 ) + val_t( 8.1609570512920303e-035 ),
    val_t( 2.4867833374282006e-003 ) + val_t( 2.1393364986507696e-019 ) + val_t( 1.4052965094417890e-035 ),
    val_t( 1.2433964118467430e-003 ) + val_t( 1.5687355176030872e-019 ) + val_t( 7.8038942591233641e-037 ),
    val_t( 6.2169879882004067e-004 ) + val_t( 8.8052478855119078e-020 ) + val_t( 2.9796757779118067e-038 ),
    val_t( 3.1084947352226304e-004 ) + val_t( 6.3823189926259110e-021 ) + val_t( 2.1568866924378185e-037 ),
    val_t( 1.5542474602516681e-004 ) + val_t( 2.1425504083516774e-020 ) + val_t( 2.5972422573637112e-036 ),
    val_t( 7.7712374170587977e-005 ) + val_t( 5.6622060858042360e-021 ) + val_t( 3.4585505779559275e-037 ),
    val_t( 3.8856187230044567e-005 ) + val_t( 2.7666224780292482e-022 ) + val_t( 8.4307171707724591e-039 ),
    val_t( 1.9428093633116103e-005 ) + val_t( 2.5117855297401599e-021 ) + val_t( 1.9848321970243239e-037 ),
    val_t( 9.7140468188197804e-006 ) + val_t( 2.8676454050134809e-022 ) + val_t( 7.3043555191897276e-039 ),
    val_t( 4.8570234096926061e-006 ) + val_t( 2.3414758513676060e-022 ) + val_t( 1.5639401814228348e-038 ),
    val_t( 2.4285117048816425e-006 ) + val_t( 1.2842408524535048e-022 ) + val_t( 2.2024333405590927e-038 ),
    val_t( 1.2142558524452387e-006 ) + val_t( 1.2691414638784321e-023 ) + val_t( 4.9599747950222456e-040 ),
    val_t( 6.0712792622317153e-007 ) + val_t( 2.6375412942942729e-023 ) + val_t( 4.7921955589759873e-040 ),
    val_t( 3.0356396311165480e-007 ) + val_t( 2.4565300148117443e-024 ) + val_t( 8.8673268886368927e-041 ),
    val_t( 1.5178198155583600e-007 ) + val_t( 2.6356647556434210e-023 ) + val_t( 1.5810643238435611e-039 ),
    val_t( 7.5890990777919087e-008 ) + val_t( 6.3932042463472506e-024 ) + val_t( 3.9699174755870367e-041 ),
    val_t( 3.7945495388959676e-008 ) + val_t( 5.6571846319063139e-024 ) + val_t( 7.2520095114878692e-040 ),
    val_t( 1.8972747694479854e-008 ) + val_t( 3.1361651295448778e-024 ) + val_t( 3.3799132328810282e-040 ),
    val_t( 9.4863738472399305e-009 ) + val_t( 3.6575824764186687e-025 ) + val_t( 1.3162521577894960e-041 ),
    val_t( 4.7431869236199653e-009 ) + val_t( 4.4617889045612585e-025 ) + val_t( 3.8290717626005075e-041 ),
    val_t( 2.3715934618099826e-009 ) + val_t( 2.5600191605746197e-025 ) + val_t( 3.2959530005120734e-041 ),
    val_t( 1.1857967309049913e-009 ) + val_t( 1.3211501688240587e-025 ) + val_t( 1.8334997982125505e-041 ),
    val_t( 5.9289836545249566e-010 ) + val_t( 6.6571765797912293e-026 ) + val_t( 9.4034175379005928e-042 ),
    val_t( 2.9644918272624783e-010 ) + val_t( 3.3350165068544821e-026 ) + val_t( 4.2653515566144493e-043 ),
    val_t( 1.4822459136312391e-010 ) + val_t( 1.6683117805470994e-026 ) + val_t( 3.9633961024024284e-043 ),
    val_t( 7.4112295681561957e-011 ) + val_t( 8.3425633116353196e-027 ) + val_t( 5.7978633854897459e-043 ),
    val_t( 3.7056147840780979e-011 ) + val_t( 4.1714072069301373e-027 ) + val_t( 6.9632764664870052e-043 ),
    val_t( 1.8528073920390489e-011 ) + val_t( 2.0857192973541284e-027 ) + val_t( 3.0928503139897782e-043 ),
    val_t( 9.2640369601952446e-012 ) + val_t( 1.0428616104131968e-027 ) + val_t( 1.5258014137373641e-044 ),
    val_t( 4.6320184800976223e-012 ) + val_t( 5.2143105042361493e-028 ) + val_t( 3.5047495231933390e-044 ),
    val_t( 2.3160092400488112e-012 ) + val_t( 2.6071555586393451e-028 ) + val_t( 4.3371834065573239e-044 ),
    val_t( 1.1580046200244056e-012 ) + val_t( 1.3035778176348314e-028 ) + val_t( 1.9311733981688285e-044 ),
    val_t( 5.7900231001220279e-013 ) + val_t( 6.5178891360681059e-029 ) + val_t( 6.5564971808072191e-045 ),
    val_t( 2.8950115500610139e-013 ) + val_t( 3.2589445740207969e-029 ) + val_t( 8.8230435499360671e-047 ),
    val_t( 1.4475057750305070e-013 ) + val_t( 1.6294722877587411e-029 ) + val_t( 2.4479598770362830e-045 ),
    val_t( 7.2375288751525349e-014 ) + val_t( 8.1473614397291355e-030 ) + val_t( 1.2316205660414995e-046 ),
    val_t( 3.6187644375762674e-014 ) + val_t( 4.0736807199814963e-030 ) + val_t( 9.9141101103428151e-047 ),
    val_t( 1.8093822187881337e-014 ) + val_t( 2.0368403600053640e-030 ) + val_t( 2.2942786769248533e-046 ),
    val_t( 9.0469110939406686e-015 ) + val_t( 1.0184201800045091e-030 ) + val_t( 5.8243674583874872e-048 ),
    val_t( 4.5234555469703343e-015 ) + val_t( 5.0921009000248289e-031 ) + val_t( 5.4986853445937639e-047 ),
    val_t( 2.2617277734851671e-015 ) + val_t( 2.5460504500127000e-031 ) + val_t( 2.3055116185024174e-047 ),
    val_t( 1.1308638867425836e-015 ) + val_t( 1.2730252250063857e-031 ) + val_t( 1.0972769275269006e-047 ),
    val_t( 5.6543194337129179e-016 ) + val_t( 6.3651261250319733e-032 ) + val_t( 2.6801249723447102e-048 ),
    val_t( 2.8271597168564589e-016 ) + val_t( 3.1825630625159921e-032 ) + val_t( 2.3577355595783352e-048 ),
    val_t( 1.4135798584282295e-016 ) + val_t( 1.5912815312579966e-032 ) + val_t( 2.6745324455321193e-048 ),
    val_t( 7.0678992921411473e-017 ) + val_t( 7.9564076562899844e-033 ) + val_t( 8.3999654020032640e-049 ),
    val_t( 3.5339496460705737e-017 ) + val_t( 3.9782038281449922e-033 ) + val_t( 5.2889650122534711e-049 ),
    val_t( 1.7669748230352868e-017 ) + val_t( 1.9891019140724961e-033 ) + val_t( 2.7806052950332153e-049 ),
    val_t( 8.8348741151764342e-018 ) + val_t( 9.9455095703624804e-034 ) + val_t( 1.4073179961299176e-049 ),
    val_t( 4.4174370575882171e-018 ) + val_t( 4.9727547851812402e-034 ) + val_t( 7.0578591664162261e-050 ),
    val_t( 2.2087185287941085e-018 ) + val_t( 2.4863773925906201e-034 ) + val_t( 3.5315882314289427e-050 ),
    val_t( 1.1043592643970543e-018 ) + val_t( 1.2431886962953101e-034 ) + val_t( 1.7661264467420749e-050 ),
    val_t( 5.5217963219852714e-019 ) + val_t( 6.2159434814765503e-035 ) + val_t( 8.8310476474948791e-051 ),
    val_t( 2.7608981609926357e-019 ) + val_t( 3.1079717407382751e-035 ) + val_t( 4.4155757504705028e-051 ),
    val_t( 1.3804490804963178e-019 ) + val_t( 1.5539858703691376e-035 ) + val_t( 2.2077943660756344e-051 ),
    val_t( 6.9022454024815892e-020 ) + val_t( 7.7699293518456879e-036 ) + val_t( 1.1038979943928650e-051 ),
    val_t( 3.4511227012407946e-020 ) + val_t( 3.8849646759228439e-036 ) + val_t( 5.5194909861581346e-052 ),
    val_t( 1.7255613506203973e-020 ) + val_t( 1.9424823379614220e-036 ) + val_t( 2.7597456198532936e-052 ),
    val_t( 8.6278067531019865e-021 ) + val_t( 9.7124116898071098e-037 ) + val_t( 1.3798728257734252e-052 ),
    val_t( 4.3139033765509932e-021 ) + val_t( 4.8562058449035549e-037 ) + val_t( 6.8993641486755980e-053 ),
    val_t( 2.1569516882754966e-021 ) + val_t( 2.4281029224517775e-037 ) + val_t( 3.4496820768138582e-053 ),
    val_t( 1.0784758441377483e-021 ) + val_t( 1.2140514612258887e-037 ) + val_t( 1.7248410387164366e-053 ),
    val_t( 5.3923792206887416e-022 ) + val_t( 6.0702573061294436e-038 ) + val_t( 8.6242051939690673e-054 ),
    val_t( 2.6961896103443708e-022 ) + val_t( 3.0351286530647218e-038 ) + val_t( 4.3121025970328942e-054 ),
    val_t( 1.3480948051721854e-022 ) + val_t( 1.5175643265323609e-038 ) + val_t( 2.1560512985224922e-054 ),
    val_t( 6.7404740258609269e-023 ) + val_t( 7.5878216326618045e-039 ) + val_t( 1.0780256492620017e-054 ),
    val_t( 3.3702370129304635e-023 ) + val_t( 3.7939108163309023e-039 ) + val_t( 5.3901282463109533e-055 ),
    val_t( 1.6851185064652317e-023 ) + val_t( 1.8969554081654511e-039 ) + val_t( 2.6950641231555944e-055 ),
    val_t( 8.4255925323261587e-024 ) + val_t( 9.4847770408272557e-040 ) + val_t( 1.3475320615778120e-055 ),
    val_t( 4.2127962661630793e-024 ) + val_t( 4.7423885204136278e-040 ) + val_t( 6.7376603078890783e-056 ),
    val_t( 2.1063981330815397e-024 ) + val_t( 2.3711942602068139e-040 ) + val_t( 3.3688301539445414e-056 ),
    val_t( 1.0531990665407698e-024 ) + val_t( 1.1855971301034070e-040 ) + val_t( 1.6844150769722712e-056 ),
    val_t( 5.2659953327038492e-025 ) + val_t( 5.9279856505170348e-041 ) + val_t( 8.4220753848613558e-057 ),
    val_t( 2.6329976663519246e-025 ) + val_t( 2.9639928252585174e-041 ) + val_t( 4.2110376924306779e-057 ),
    val_t( 1.3164988331759623e-025 ) + val_t( 1.4819964126292587e-041 ) + val_t( 2.1055188462153389e-057 ),
    val_t( 6.5824941658798115e-026 ) + val_t( 7.4099820631462935e-042 ) + val_t( 1.0527594231076695e-057 ),
    val_t( 3.2912470829399057e-026 ) + val_t( 3.7049910315731467e-042 ) + val_t( 5.2637971155383474e-058 ),
    val_t( 1.6456235414699529e-026 ) + val_t( 1.8524955157865734e-042 ) + val_t( 2.6318985577691737e-058 ),
    val_t( 8.2281177073497643e-027 ) + val_t( 9.2624775789328669e-043 ) + val_t( 1.3159492788845868e-058 ),
    val_t( 4.1140588536748822e-027 ) + val_t( 4.6312387894664334e-043 ) + val_t( 6.5797463944229342e-059 ),
    val_t( 2.0570294268374411e-027 ) + val_t( 2.3156193947332167e-043 ) + val_t( 3.2898731972114671e-059 ),
    val_t( 1.0285147134187205e-027 ) + val_t( 1.1578096973666084e-043 ) + val_t( 1.6449365986057335e-059 ),
    val_t( 5.1425735670936027e-028 ) + val_t( 5.7890484868330418e-044 ) + val_t( 8.2246829930286677e-060 ),
    val_t( 2.5712867835468014e-028 ) + val_t( 2.8945242434165209e-044 ) + val_t( 4.1123414965143339e-060 ),
    val_t( 1.2856433917734007e-028 ) + val_t( 1.4472621217082604e-044 ) + val_t( 2.0561707482571669e-060 ),
    val_t( 6.4282169588670034e-029 ) + val_t( 7.2363106085413022e-045 ) + val_t( 1.0280853741285835e-060 ),
    val_t( 3.2141084794335017e-029 ) + val_t( 3.6181553042706511e-045 ) + val_t( 5.1404268706429173e-061 ),
    val_t( 1.6070542397167508e-029 ) + val_t( 1.8090776521353256e-045 ) + val_t( 2.5702134353214587e-061 ),
    val_t( 8.0352711985837542e-030 ) + val_t( 9.0453882606766278e-046 ) + val_t( 1.2851067176607293e-061 ),
    val_t( 4.0176355992918771e-030 ) + val_t( 4.5226941303383139e-046 ) + val_t( 6.4255335883036467e-062 ),
    val_t( 2.0088177996459386e-030 ) + val_t( 2.2613470651691570e-046 ) + val_t( 3.2127667941518233e-062 ),
    val_t( 1.0044088998229693e-030 ) + val_t( 1.1306735325845785e-046 ) + val_t( 1.6063833970759117e-062 ),
    val_t( 5.0220444991148464e-031 ) + val_t( 5.6533676629228924e-047 ) + val_t( 8.0319169853795583e-063 ),
    val_t( 2.5110222495574232e-031 ) + val_t( 2.8266838314614462e-047 ) + val_t( 4.0159584926897792e-063 ),
    val_t( 1.2555111247787116e-031 ) + val_t( 1.4133419157307231e-047 ) + val_t( 2.0079792463448896e-063 ),
    val_t( 6.2775556238935580e-032 ) + val_t( 7.0667095786536155e-048 ) + val_t( 1.0039896231724448e-063 ),
    val_t( 3.1387778119467790e-032 ) + val_t( 3.5333547893268077e-048 ) + val_t( 5.0199481158622240e-064 ),
    val_t( 1.5693889059733895e-032 ) + val_t( 1.7666773946634039e-048 ) + val_t( 2.5099740579311120e-064 ),
    val_t( 7.8469445298669475e-033 ) + val_t( 8.8333869733170193e-049 ) + val_t( 1.2549870289655560e-064 ),
    val_t( 3.9234722649334737e-033 ) + val_t( 4.4166934866585097e-049 ) + val_t( 6.2749351448277800e-065 ),
    val_t( 1.9617361324667369e-033 ) + val_t( 2.2083467433292548e-049 ) + val_t( 3.1374675724138900e-065 ),
    val_t( 9.8086806623336844e-034 ) + val_t( 1.1041733716646274e-049 ) + val_t( 1.5687337862069450e-065 ),
    val_t( 4.9043403311668422e-034 ) + val_t( 5.5208668583231371e-050 ) + val_t( 7.8436689310347249e-066 ),
    val_t( 2.4521701655834211e-034 ) + val_t( 2.7604334291615685e-050 ) + val_t( 3.9218344655173625e-066 ),
    val_t( 1.2260850827917105e-034 ) + val_t( 1.3802167145807843e-050 ) + val_t( 1.9609172327586812e-066 ),
    val_t( 6.1304254139585527e-035 ) + val_t( 6.9010835729039214e-051 ) + val_t( 9.8045861637934062e-067 ),
    val_t( 3.0652127069792764e-035 ) + val_t( 3.4505417864519607e-051 ) + val_t( 4.9022930818967031e-067 ),
    val_t( 1.5326063534896382e-035 ) + val_t( 1.7252708932259803e-051 ) + val_t( 2.4511465409483515e-067 ),
    val_t( 7.6630317674481909e-036 ) + val_t( 8.6263544661299017e-052 ) + val_t( 1.2255732704741758e-067 ),
    val_t( 3.8315158837240955e-036 ) + val_t( 4.3131772330649509e-052 ) + val_t( 6.1278663523708789e-068 ),
    val_t( 1.9157579418620477e-036 ) + val_t( 2.1565886165324754e-052 ) + val_t( 3.0639331761854394e-068 ),
    val_t( 9.5787897093102386e-037 ) + val_t( 1.0782943082662377e-052 ) + val_t( 1.5319665880927197e-068 ),
    val_t( 4.7893948546551193e-037 ) + val_t( 5.3914715413311886e-053 ) + val_t( 7.6598329404635986e-069 ),
    val_t( 2.3946974273275597e-037 ) + val_t( 2.6957357706655943e-053 ) + val_t( 3.8299164702317993e-069 ),
    val_t( 1.1973487136637798e-037 ) + val_t( 1.3478678853327971e-053 ) + val_t( 1.9149582351158996e-069 ),
    val_t( 5.9867435683188992e-038 ) + val_t( 6.7393394266639857e-054 ) + val_t( 9.5747911755794982e-070 ),
    val_t( 2.9933717841594496e-038 ) + val_t( 3.3696697133319929e-054 ) + val_t( 4.7873955877897491e-070 ),
    val_t( 1.4966858920797248e-038 ) + val_t( 1.6848348566659964e-054 ) + val_t( 2.3936977938948746e-070 ),
    val_t( 7.4834294603986239e-039 ) + val_t( 8.4241742833299821e-055 ) + val_t( 1.1968488969474373e-070 ),
    val_t( 3.7417147301993120e-039 ) + val_t( 4.2120871416649911e-055 ) + val_t( 5.9842444847371864e-071 ),
    val_t( 1.8708573650996560e-039 ) + val_t( 2.1060435708324955e-055 ) + val_t( 2.9921222423685932e-071 ),
    val_t( 9.3542868254982799e-040 ) + val_t( 1.0530217854162478e-055 ) + val_t( 1.4960611211842966e-071 )
  };

  template < int ZW >
  static ac_fixed<ZW, 0, false> x2_asin_pi_2mi(int i)
  {
    MgcAcIarcsinAssert< ZW <= 128 >::test();
    // Assume no more than 130 entries.
    #pragma hls_waive CNS

    if (i > 129)
    { return 0.0; }
    return x2_asin_pi_pow2_table[i];
  }

  //============================================================================
  // Function:  theta = arcsin(t) (for ac_fixed)
  // Inputs:
  //   - argument t in range [-1, 1]
  // Outputs:
  //   - arcsin, inverse sine angle in radians, scaled by 1/Pi
  //-------------------------------------------------------------------------

  template< int AW, int AI, ac_q_mode AQ, ac_o_mode AO,
            int ZW, int ZI, ac_q_mode ZQ, ac_o_mode ZO >
  void ac_arcsin_cordic(ac_fixed<AW,AI,true,AQ,AO> t, ac_fixed<ZW,ZI,true,ZQ,ZO> &arcsin)
  {
    const int P_F = 2*(ZW - ZI);
    const int GUARD = 2;
    const int N_I = P_F + 2;
    MgcAcIarcsinAssert< P_F <= 128 >::test();

    typedef ac_fixed<P_F+3+GUARD,3,true> dp_t;
    typedef ac_fixed<P_F+1,1,true> dp_theta_t;

    dp_t arg_t = t;
    dp_t abs_t;
    if (t < 0) {
      abs_t = -t;
    } else {
      abs_t = t;
    }
    dp_t x = 1.0;
    dp_t y = 0.0;
    dp_t tn = abs_t;
    dp_theta_t theta = 0.0;

    for (int i = 0; i < N_I; i++) {
      bool sel = (y <= tn && x >= 0) || (y > tn && x < 0);
      #pragma hls_waive CNS
      if (P_F > 20) {
        // Exhaustively tested cases for P_F <= 20 pass error criteria without
        // this check.
        sel = sel && (theta <= 0.5);
      }
      #pragma hls_waive ISE
      dp_t x_d = y >> (i - 1);
      #pragma hls_waive ISE
      dp_t y_d = x >> (i - 1);
      dp_theta_t theta_d = x2_asin_pi_2mi<P_F>(i);
      #pragma hls_waive ISE
      dp_t tn_d = tn >> (2*i);
      #pragma hls_waive ISE
      x -= x >> (2*i);
      #pragma hls_waive ISE
      y -= y >> (2*i);
      if (sel) {
        x -= x_d;
        y += y_d;
        theta += theta_d;
      } else {
        x += x_d;
        y -= y_d;
        theta -= theta_d;
      }
      tn += tn_d;
    }
    if ((theta < 0) != (t < 0)) {
      arcsin = -theta;
    } else {
      arcsin = theta;
    }
  }

  //============================================================================
  // Function:  theta = arcsin(t) (for ac_float)
  // Inputs:
  //   - argument t in range [-1, 1]
  // Outputs:
  //   - arcsin, inverse sine angle in radians, scaled by 1/Pi
  //-------------------------------------------------------------------------

  template< int AW, int AI, int AE, ac_q_mode AQ,
            int OW, int OI, int OE, ac_q_mode OQ >
  void ac_arcsin_cordic(
    const ac_float<AW,AI,AE,AQ> &t,
    ac_float<OW,OI,OE,OQ> &arcsin
  )
  {
    ac_fixed<AW, AI, true> mantVal = t.mantissa();
    int exp_val = t.exp().to_int();
    // Intermediate ac_fixed variables to store the value of inputs and outputs
    // and enable compatibility with ac_fixed implementation.
    ac_fixed<20, 2, true, AC_RND> input_fi;
    // Use ac_shift_left instead of "<<" operator to ensure rounding.
    ac_math::ac_shift_left(mantVal, exp_val, input_fi);
    ac_fixed<35, 5, true> arcsin_output_fi;
    ac_arcsin_cordic(input_fi, arcsin_output_fi);
    // Convert ac_fixed output to ac_float by using a constructor.
    ac_float<OW,OI,OE,OQ> output_temp(arcsin_output_fi);

    arcsin = output_temp;
  }

//=========================================================================
// Function: ac_arcsin_cordic (for ac_std_float)
//
//-------------------------------------------------------------------------

  template <int W, int E, int outW, int outE>
  void ac_arcsin_cordic(
    const ac_std_float<W, E> &t,
    ac_std_float<outW, outE> &arcsin
  )
  {
    ac_float<outW - outE + 1, 2, outE> arcsin_ac_fl; // Equivalent ac_float representation for output.
    ac_arcsin_cordic(t.to_ac_float(), arcsin_ac_fl); // Call ac_float version.
    ac_std_float<outW, outE> output_temp(arcsin_ac_fl); // Convert output ac_float to ac_std_float.
    arcsin = output_temp;
  }

//=========================================================================
// Function: ac_arcsin_cordic (for ac_ieee_float)
//
//-------------------------------------------------------------------------

  template<ac_ieee_float_format Format,
           ac_ieee_float_format outFormat>
  void ac_arcsin_cordic(
    const ac_ieee_float<Format> &t,
    ac_ieee_float<outFormat> &arcsin
  )
  {
    typedef ac_ieee_float<outFormat> T_out;
    const int outW = T_out::width;
    const int outE = T_out::e_width;
    ac_float<outW - outE + 1, 2, outE> arcsin_ac_fl; // Equivalent ac_float representation for output.
    ac_arcsin_cordic(t.to_ac_float(), arcsin_ac_fl); // Call ac_float version.
    ac_ieee_float<outFormat> output_temp(arcsin_ac_fl); // Convert output ac_float to ac_ieee_float.
    arcsin = output_temp;
  }

} // namespace ac_math

#endif

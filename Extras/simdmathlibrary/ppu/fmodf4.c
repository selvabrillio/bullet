/* fmodf4 - for each of four float slots, compute remainder of x/y defined as x - truncated_integer(x/y) * y.
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
 */

#include <simdmath.h>
#include <altivec.h>

#include "common-types.h"

//
// This returns an accurate result when |divf4(x,y)| < 2^20 and |x| < 2^128, and otherwise returns zero.
// If x == 0, the result is 0.
// If x != 0 and y == 0, the result is undefined.

vector float
fmodf4 (vector float x, vector float y)
{
   vec_float4 q, xabs, yabs, qabs, xabs2;
   vec_int4   qi0, qi1, qi2;
   vec_float4 i0, i1, i2, r1, r2, i;
   vec_uint4  inrange;
 
   // Find i = truncated_integer(|x/y|)
 
   // If |divf4(x,y)| < 2^20, the quotient is at most off by 1.0.
   // Thus i is either the truncated quotient, one less, or one greater.
                                                                                
   q = divf4( x, y );
   xabs = fabsf4( x );
   yabs = fabsf4( y );
   qabs = fabsf4( q );
   xabs2 = vec_add( xabs, xabs );
                                                                                
   inrange = (vec_uint4)vec_cmpgt( (vec_float4)(vec_splatsu4(0x49800000)), qabs );
                                                                                
   qi1 = vec_cts( qabs, 0 );
   qi0 = vec_add( qi1, ((vec_int4){-1, -1, -1, -1}) );
   qi2 = vec_add( qi1, ((vec_int4){1, 1, 1, 1}) );
                                                                                                                                                          
   i0 = vec_ctf( qi0, 0 );
   i1 = vec_ctf( qi1, 0 );
   i2 = vec_ctf( qi2, 0 );
                                                                                                                                                          
   // Correct i will be the largest one such that |x| - i*|y| >= 0.
                                                                                                                                                          
   r1 = vec_nmsub( i1, yabs, xabs );
   r2 = vec_nmsub( i2, yabs, xabs );
                                                                                                                                                          
   i = i0;
   i = vec_sel( i1, i, vec_cmpgt( vec_splatsi4(0), (vec_int4)r1 ) );
   i = vec_sel( i2, i, vec_cmpgt( vec_splatsi4(0), (vec_int4)r2 ) );
                                                                                                                                                          
   i = copysignf4( i, q );
                                                                                                                                                          
   return vec_sel( vec_splatsf4(0.0f), vec_nmsub( i, y, x ), inrange );
}


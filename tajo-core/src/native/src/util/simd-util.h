// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TAJO_UTIL_SIMD_UTIL_H_
#define TAJO_UTIL_SIMD_UTIL_H_

#ifdef TAJO_INTEL_SPECIFIED_INTRINSICS_HEADER
#include <mmintrin.h>   // intrinsics for MMX instructions
#include <xmmintrin.h>  // intrinsics for SSE instructions
#include <emmintrin.h>  // intrinsics for SSE2 instructions
#include <pmmintrin.h>  // intrinsics for SSE3 instructions
#include <tmmintrin.h>  // intrinsics for SSSE3 instructions
#include <smmintrin.h>  // intrinsics for SSE4 instructions
#include <nmmintrin.h>  // intrinsics for SSE4.2 instructions
#include <ammintrin.h>  // intrinsics for SSE4A instructions
#include <wmmintrin.h>  // intrinsics for AES instructions
#include <immintrin.h>  // intrinsics for AVX instructions
#endif

#include <x86intrin.h>  // Meta header for Intel MMX, SSE and AVX Intrinsics

namespace tajo {

namespace SSE {

static const int SIZE_OF_m64 = 8;    // _mm64
static const int SIZE_OF_m128 = 16;  // _mm128
static const int SIZE_OF_m256 = 23;  // _mm256

// For Intel SSE 4.2 - SSE4.2 String and Text Processing Instructions (STTNI)
//
// Please refer to Chapter 10, Intel 64 and IA-32 Architectures Optimization Reference Manual
// at http://www.intel.com/content/dam/doc/manual/64-ia-32-architectures-optimization-manual.pdf.
//
// Also, the following is useful references for Intel intrinsics.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/

static const int STRCHR_FIRST = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;
static const int STRCHR_LAST  = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_POSITIVE_POLARITY | _SIDD_MOST_SIGNIFICANT;

// This mode will return the first index of different bytes.
static const int STR_EQUAL = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY;

// This mode will return the first index of different bytes.
static const int STRSTR = _SIDD_UBYTE_OPS  | _SIDD_CMP_RANGES | _SIDD_LEAST_SIGNIFICANT;

} // namespace tajo::SSE

} // namespace tajo
#endif /* TAJO_UTIL_SIMD_UTIL_H_ */

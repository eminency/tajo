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

#include <algorithm>
#include <cstring>
#include <string>
#include <immintrin.h>

#include "glog/logging.h"

#include "common/stringpiece.h"


using namespace std;

namespace tajo {

  const StringPiece StringPiece::Slice(const int offset, const int len) const {
    DCHECK((offset + len) <= len_) << "ERROR: slice cannot be out of the src string. (size: " << len_
        << ") but, slice wants to be (" << offset << ", " << (offset + len);

    return StringPiece(ptr_ + offset, len);
  }

  bool StringPiece::operator==(const StringPiece &other) const {

#ifdef __SSE4_2__
  char* str1 = const_cast<char*>(ptr_);
  char* str2 = const_cast<char*>(other.data());

  __m128i s1, s2;

  int c, result = 0, len = len_;

  do {
    s1 = _mm_lddqu_si128((const __m128i*) str1);
    s2 = _mm_lddqu_si128((const __m128i*) str2);
    c = _mm_cmpestri(s1, len - result, s2, 16, _SIDD_CMP_EQUAL_EACH | _SIDD_MASKED_NEGATIVE_POLARITY);

    str1 += 16;
    str2 += 16;

    result += c;

  } while(c == 16);

  return ((result==len) ? result : 0);
#else
    return (this == &other) ||
        ((length() == other.length()) && memcmp(data(), other.data(), std::min(length(), other.length())) == 0);
#endif
  }

  bool StringPiece::operator!=(const StringPiece &other) const {
    return (length() != other.length()) || memcmp(data(), other.data(), std::min(length(), other.length())) != 0;
  }

  std::ostream &operator<<(std::ostream &os, const StringPiece &str) {
    return os << str.ToString();
  }
}

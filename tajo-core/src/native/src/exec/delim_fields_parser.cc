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

#include "common/stringpiece.h"
#include "util/simd-util.h"
#include "delim_fields_parser.h"

namespace tajo {

class StringPiece;

DelimFieldsParser::DelimFieldsParser(const char field_delim)
    : field_delim_(field_delim) {

  char field_delim_m128[SSE::SIZE_OF_m128];
  memset(field_delim_m128, 0, SSE::SIZE_OF_m128);
  field_delim_m128[0] = field_delim_;

  this->sse_field_delim_ = _mm_load_si128(reinterpret_cast<const __m128i *>(field_delim_m128));
}

void DelimFieldsParser::ParseFields(StringPiece *line,      /**< a delimited line to be parsed      */
    StringPiece fields[],   /**< parsed fields to be returned       */
    int fields_num,         /**< the number of fields object        */
    int &actual_fields_num  /**< the number of actual parsed fields */
) {
  const int len = line->length();

  int curPos = 0;
  char *str = const_cast<char *>(line->data());
  int fieldIdx = 0;
  int lastFieldOffset = 0;

#ifdef __SSE4_2__
  __m128i sse_buf;
  int r;

  // if the remain length is larger than SSE register size (16 bytes) and
  // the current field is not less than intended fields num.
  while((len - curPos) >= SSE::SIZE_OF_m128 && fieldIdx < fields_num) {
    sse_buf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
    r = _mm_cmpestri(this->sse_field_delim_, 1, sse_buf, 16, SSE::STRCHR_FIRST);

    if (r < 16) {
      fields[fieldIdx].Set(line->data() + lastFieldOffset, (curPos + r) - lastFieldOffset);
      r++; // for skipping the field delimiter.
      lastFieldOffset = curPos + r;
      fieldIdx++;
    }

    str += r;
    curPos += r;
  }
#endif

  for (; curPos < len && fieldIdx < fields_num; ++curPos) {
    char c = *str++;

    // It will be one if this char is the final char.
    bool finalFlag = curPos == (len - 1) && *(str - 1) != this->field_delim_;

    if (c == this->field_delim_ || finalFlag) {
      fields[fieldIdx].Set(line->data() + lastFieldOffset, curPos - lastFieldOffset + finalFlag);
      lastFieldOffset = curPos + 1;
      fieldIdx++;
    }
  }

  actual_fields_num = fieldIdx;
}

} /* namespace tajo */

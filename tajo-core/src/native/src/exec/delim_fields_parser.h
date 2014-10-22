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

#ifndef TAJO_EXEC_DELIMITED_FIELDS_PARSER_H_
#define TAJO_EXEC_DELIMITED_FIELDS_PARSER_H_

#include "util/simd-util.h"

namespace tajo {

struct StringPiece;

// Designed and implemented by Hyunsik
//
// Zero-copy field delimiter parser implementation. It parses one line into
// multiple fields. It is optimized in x86_64 SSE 4.2,

class DelimFieldsParser {
 public:
  DelimFieldsParser(const char field_delim);

  ~DelimFieldsParser() {
  }

  void ParseFields(StringPiece *line, StringPiece fields[], int fields_num, int &actual_fields_num);

 private:
  const char field_delim_;
  __m128i sse_field_delim_;
};

} /* namespace tajo */

#endif /* TAJO_EXEC_DELIMITED_FIELDS_PARSER_H_ */

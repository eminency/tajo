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

#ifndef TAJO_STORAGE_TEXT_SCANNER_H_
#define TAJO_STORAGE_TEXT_SCANNER_H_

#include "scanner.h"
#include "common/datatypes.h"
#include "exec/delim_fields_parser.h"
#include "io/stream.h"
#include "util/simd-util.h"

namespace tajo {

class BufferAllocator;

class Schema;

class Column;

class VecRowBlock;

// Designed and implemented by Hyunsik
//
// It is a zero-copy line delimited scanner. The main objective is to read
// each line and than parses the line by using fields_parser.
// Line parse implementation is optimized in SSE 4.2 STTNI.

class TextScanner : public Exec {

 public:
  static const int READ_BUF_SIZE = 65536;

  TextScanner(BufferAllocator *alloc, Stream *stream, Schema &schema,
      const char line_delim);

  virtual ~TextScanner() {
  };

  void DoInit();

  bool DoGetNext(VecRowBlock *vrows);

  void DoReset();

  void DoClose();

  void SetTargets(const std::vector<Column> &targets) {
  }

  inline const Schema &schema() const {
    return schema_;
  }

  int FindFirstRecordIndex(const char *buf, size_t len);

  int NextLineDelimIndex(
      const char *buf,
      size_t len,
      int *delim_idx_list,
      int list_num,
      int &found_line_num);

 private:
  BufferAllocator *alloc_;
  Stream *stream_;
  const Schema &schema_;
  const char line_delim_;
  DelimFieldsParser fields_parser_;

  char buffer_[READ_BUF_SIZE];
  int delim_idx_list[1024];


  // for SSE4.2
  __m128i sse_line_delim_;

  int64_t offset_;
  int64_t buffer_len_;
  int64_t last_read_len_;

  // Should replace it by InputSplit
  int64_t start_offset_;
  int64_t legnth_;
};

}


#endif /* TAJO_STORAGE_TEXT_SCANNER_H_ */

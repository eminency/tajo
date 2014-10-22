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

#include <iostream>
#include "vector/vectors.h"
#include "exec/text_scanner.h"
#include "util/simd-util.h"

namespace tajo {

TextScanner::TextScanner(BufferAllocator *alloc, Stream *stream, Schema &schema, const char line_delim)
    : alloc_(alloc),
      stream_(stream),
      schema_(schema),
      line_delim_(line_delim),
      fields_parser_('|') {

  char line_delim_m128[SSE::SIZE_OF_m128];
  memset(line_delim_m128, 0, SSE::SIZE_OF_m128);
  line_delim_m128[0] = line_delim_;

  this->sse_line_delim_ = _mm_load_si128(reinterpret_cast<const __m128i *> (line_delim_m128));

  // TODO
  this->start_offset_ = 0;
  this->legnth_ = this->stream_->length();
};

void TextScanner::DoInit() {
  memset(buffer_, 0, READ_BUF_SIZE);
  stream_->Open();

  last_read_len_ = stream_->Read(buffer_, READ_BUF_SIZE);
  if (start_offset_ > 0) { // if it is not begin of a file
    while (last_read_len_ > 0 && (offset_ = FindFirstRecordIndex(buffer_, last_read_len_)) < 0) {
      last_read_len_ = stream_->Read(buffer_, READ_BUF_SIZE);
    }
  } else {
    offset_ = 0;
  }
  buffer_len_ = last_read_len_;
}

bool TextScanner::DoGetNext(VecRowBlock *vrows) {
  int cur_row_num = 0;
  int found_line_num = 0;
  StringPiece line;
  int col_num = this->schema_.size();
  StringPiece *field_strs = new StringPiece[col_num];

  char *line_start;
  int prev_len;
  int filled_col_num;

  do {

    //////////////////////////////////////
    // Splitting buffer by line delimiter
    //////////////////////////////////////

    // There are three cases about last_delim_idx value:
    // * case 1: last_delim_idx == READ_BUFFER_SIZE - 1
    // * case 2: last_delim_idx < READ_BUFFER_SIZE - 1
    // * case 3: last_delim_idx = -1
    int last_delim_idx = NextLineDelimIndex(buffer_, buffer_len_, delim_idx_list, vrows->VectorSize(), found_line_num);
    cur_row_num += found_line_num;

    //////////////////////////////////////
    // Filling VecRowBlock
    //////////////////////////////////////

    line_start = buffer_;
    prev_len = -1;
    for (int i = 0; i < found_line_num; i++) {
      line.Set(line_start, delim_idx_list[i] - prev_len - 1);
      line_start = buffer_ + delim_idx_list[i] + 1;
      prev_len = delim_idx_list[i];

      fields_parser_.ParseFields(&line, field_strs, col_num, filled_col_num);
//      for (int i = 0; i < filled_col_num; i++) {
//        std::cout << field_strs[i].ToString() << "|";
//      }
//      std::cout <<std::endl;
    }

    ///////////////////////////////////
    // Prepare Next Buffer Read
    ///////////////////////////////////

    // The minimum remain_len is ZERO.
    int remain_len = buffer_len_ - (last_delim_idx + 1);
    DCHECK(remain_len >= 0) << "remain_len must be either positive value or ZERO.";

    // Copy the remain bytes to the first of buffer
    if (remain_len > 0) {
      memcpy(buffer_, buffer_ + last_delim_idx + 1, remain_len);
    }
    // Adjust
    last_read_len_ = stream_->Read(buffer_ + remain_len, READ_BUF_SIZE - remain_len);
    buffer_len_ = remain_len + last_read_len_;
    DCHECK(buffer_len_ <= READ_BUF_SIZE) << "total read bytes must be fit to READ_BUF_SIZE";

    offset_ = 0;
  } while (last_read_len_ > 0 && cur_row_num < vrows->VectorSize());

  vrows->SetSelectedNum(cur_row_num);
  delete[] field_strs;
  return last_read_len_ > 0;
}

void TextScanner::DoReset() {

}

void TextScanner::DoClose() {
  stream_->Close();
  stream_ = NULL;
}

int TextScanner::FindFirstRecordIndex(const char *buf, size_t len) {
  int curPos = 0;
  bool found = false;
  char *str = const_cast<char *>(buf);

#ifdef __SSE4_2__

  int r;
  __m128i sse_buf;

  while((len - curPos) >= SSE::SIZE_OF_m128) {
    sse_buf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
    r = _mm_cmpestri(sse_line_delim_, 1, sse_buf, 16, SSE::STRCHR_FIRST);

    if (r < 16) {
      found = true;
      curPos = r;
      break;
    }

    str += r;
    curPos += r;
  }
#endif
  for (; curPos < len; ++curPos) {
    char c = *str++;
    if (c == line_delim_) {
      ++curPos;
      found = true;
      break;
    }
  }

  return found ? curPos : -1;
}

int TextScanner::NextLineDelimIndex(
    const char *buf,
    size_t len, int *
    delim_idx_list,
    int row_num,
    int &found_line_num) {

  int cur_delim_idx = 0;
  int curPos = offset_;
  char *str = const_cast<char *>(buf + curPos);

#ifdef __SSE4_2__

  int r;
  __m128i sse_buf;

  while((len - curPos) >= SSE::SIZE_OF_m128 && cur_delim_idx < row_num) {
    sse_buf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
    r = _mm_cmpestri(sse_line_delim_, 1, sse_buf, 16, SSE::STRCHR_FIRST);

    if (r < 16) {
      delim_idx_list[cur_delim_idx] = curPos + r;
      cur_delim_idx++;

      str += (r + 1);
      curPos += (r + 1);
    } else {
      str += SSE::SIZE_OF_m128;
      curPos += SSE::SIZE_OF_m128;
    }
  }
#endif

  for (; curPos < len && cur_delim_idx < row_num; ++curPos) {
    char c = *str++;
    if (c == line_delim_) {
      delim_idx_list[cur_delim_idx] = curPos;
      cur_delim_idx++;
    }
  }

  found_line_num = cur_delim_idx;

  return found_line_num > 0 ? delim_idx_list[found_line_num - 1] : -1;
}

} // namespace tajo

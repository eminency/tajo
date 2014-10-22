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

#ifndef TAJO_IO_IN_MEM_STREAM_HPP_
#define TAJO_IO_IN_MEM_STREAM_HPP_

#include "io/stream.h"
#include "util/memory.h"

namespace tajo {

class InMemStream : public Stream {

 public:
  InMemStream(const char *data, size_t length)
      : data_(data),
        cur_ptr_(const_cast<char *>(data)),
        length_(length),
        cur_pos_(0) {
  }

  InMemStream(Buffer *buf, size_t length)
      : data_(static_cast<char *>(buf->data())),
        cur_ptr_(static_cast<char *>(buf->data())),
        length_(length),
        cur_pos_(0) {
  }

  virtual ~InMemStream() {
  }

  bool Open() {
    return true;
  }

  bool Close() {
    return true;
  }

  int64_t Read(void *buf, const size_t buf_size);

  void Reset() {
    cur_pos_ = 0;
  }

  int64_t pos() {
    return cur_pos_;
  }

  int64_t length() {
    return length_;
  }

  int64_t remain() {
    return length_ - cur_pos_;
  }

  // If true, the stream has reached the end of the file.
  bool eof() {
    DCHECK_LT(cur_pos_, length_) << "Invalid State: cur_pos_ > length_";
    return cur_pos_ >= length_;
  }

 private:
  const size_t length_;
  const char *data_;
  size_t cur_pos_;
  char *cur_ptr_;

};

} /* namespace tajo */

#endif /* TAJO_IO_IN_MEM_STREAM_HPP_ */

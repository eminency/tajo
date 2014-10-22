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


#ifndef TAJO_COMMON_STRING_PIECE_H
#define TAJO_COMMON_STRING_PIECE_H

#include <string>
#include <ostream>
#include "glog/logging.h"

namespace tajo {

  // Zero-copy string object implementation which has lots of string-related
  // utility features. This class is hardware-conscious optimized using SSE4.2.

  struct StringPiece {

  private:
    const char *ptr_;
    int len_;

  public:

    StringPiece(const char *ptr) : ptr_(ptr), len_(strlen(ptr)) {
    }

    StringPiece(const char *ptr, const int len) : ptr_(ptr), len_(len) {
    }

    StringPiece() : ptr_(NULL), len_(0) {
    }

    void Set(const char *data, const int len) {
      ptr_ = data;
      len_ = len;
    }

    char operator[](int i) const {
      DCHECK_LT(i, len_);
      return ptr_[i];
    }

    const char *data() const {
      return ptr_;
    }

    const StringPiece Slice(const int offset, const int len) const;

    const std::string ToString() const {
      return std::string(ptr_, len_);
    }

    int length() const {
      return len_;
    }

    bool operator==(const StringPiece& other) const;

    bool operator!=(const StringPiece& other) const;
  };


}

#endif


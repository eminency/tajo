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

#ifndef TAJO_IO_FILE_STREAM_HPP_
#define TAJO_IO_FILE_STREAM_HPP_

#include <iostream>
#include <fstream>

#include "stream.h"
#include "util/memory.h"

namespace tajo {

class Stream;

using namespace std;

class FileStream : public Stream {

 public:
  FileStream(const char *filename) : filename_(filename), length_(-1) {
  }

  virtual ~FileStream() {
  }

  bool Open();

  bool Close();

  int64_t Read(void *buf, const size_t buf_size);

  void Reset() {
    in_.seekg(0, ios_base::beg);
  }

  int64_t length() {
    return length_;
  }

  int64_t pos() {
    return in_.tellg();
  }

  int64_t remain() {
    return length_ - in_.tellg();
  }

  // If true, the stream has reached the end of the file.
  bool eof() {
    return in_.eof();
  }

  const char *filename() const {
    return filename_;
  }

 private:
  const char *filename_;
  std::ifstream in_;
  int64_t length_;

};

}

#endif /* TAJO_IO_FILE_STREAM_HPP_ */

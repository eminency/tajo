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

#ifndef TAJO_IO_STREAM_HPP_
#define TAJO_IO_STREAM_HPP_

#include <stddef.h>
#include <stdint.h>

namespace tajo {

class Buffer;

class Stream {

 public:
  Stream() {
  }

  virtual ~Stream() {
  }

  virtual bool Open() = 0;

  virtual int64_t Read(void *buf, const size_t buf_size) = 0;

  virtual void Reset() = 0;

  virtual int64_t length() = 0;

  virtual int64_t pos() = 0;

  virtual int64_t remain() = 0;

  // If true, the stream has reached the end of the file.
  virtual bool eof() = 0;

  virtual bool Close() = 0;

 protected:;
};

}

#endif /* TAJO_IO_STREAM_HPP_ */

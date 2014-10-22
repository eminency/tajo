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

#include <string.h>

#include "util/compiler.h"
#include "io/in_mem_stream.h"

namespace tajo {

int64_t InMemStream::Read(void *buf, const size_t buf_size) {
  int64_t r = remain();

  if (UNLIKELY(r <= 0)) {
    return -1;
  }

  int64_t actual_read_len = r < buf_size ? r : buf_size;
  memcpy(buf, (data_ + cur_pos_), actual_read_len);

  cur_pos_ += actual_read_len;
  return actual_read_len;
}

} /* namespace tajo */

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

#ifndef TAJO_IO_HDFS_FILE_STREAM_HPP_
#define TAJO_IO_HDFS_FILE_STREAM_HPP_

#include <iostream>

#include "stream.h"
#include "hdfs.h"

namespace tajo {

class Stream;

using namespace std;

class HdfsFileStream : public Stream {
public:
  HdfsFileStream(hdfsFS fs, const char *path) : hdfs_(fs), file_path_(path), hdfs_file_(NULL), length_(-1) {
  }

  virtual ~HdfsFileStream() {
    if (hdfs_file_ != NULL) {
      hdfsCloseFile(hdfs_, hdfs_file_);
      hdfs_file_ = NULL;
    }
  }

  bool Open();

  bool Close();

  int64_t Read(void *buf, const size_t buf_size);

  void Reset() {
    hdfsSeek(hdfs_, hdfs_file_, 0);
  }

  int64_t length() {
    return length_;
  }

  int64_t pos() {
    return hdfsTell(hdfs_, hdfs_file_);
  }

  bool seek(int64_t &pos) {
    return hdfsSeek(hdfs_, hdfs_file_, pos) != -1;
  }

  int64_t remain() {
    return length_ - pos();
  }

  // If true, the stream has reached the end of the file.
  bool eof() {
    return false;
  }

  const char *filename() const {
    return file_path_;
  }

private:
  hdfsFS hdfs_;
  const char *file_path_;
  hdfsFile hdfs_file_;
  int64_t length_;

};
}
#endif

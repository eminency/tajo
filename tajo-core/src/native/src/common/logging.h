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

#ifndef TAJO_COMMON_LOGGING_H_
#define TAJO_COMMON_LOGGING_H_

// This is a wrapper around the glog header.  When we are compiling to IR,
// we don't want to pull in the glog headers.  Pulling them in causes linking
// issues when we try to dynamically link the codegen'd functions.
#ifdef IR_COMPILE
#include <iostream>
  #define DCHECK(condition)
  #define DCHECK_EQ(a, b)
  #define DCHECK_NE(a, b)
  #define DCHECK_GT(a, b)
  #define DCHECK_LT(a, b)
  #define DCHECK_GE(a, b)
  #define DCHECK_LE(a, b)
  // Similar to how glog defines DCHECK for release.
  #define LOG(level) while(false) std::cout
  #define VLOG(level) while(false) std::cout
#else
// GLOG defines this based on the system but doesn't check if it's already
// been defined.  undef it first to avoid warnings.
// glog MUST be included before gflags.  Instead of including them,
// our files should include this file instead.
#undef _XOPEN_SOURCE
// This is including a glog internal file.  We want this to expose the
// function to get the stack trace.
#include <glog/logging.h>
#include <gflags/gflags.h>
#endif

#define VLOG_VECTOR        VLOG(3)

#define VLOG_VECTOR_IS_ON VLOG_IS_ON(3)

#endif
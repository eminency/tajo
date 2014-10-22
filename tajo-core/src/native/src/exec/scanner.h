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

#ifndef TAJO_STORAGE_SCANNER_H_
#define TAJO_STORAGE_SCANNER_H_

#include <vector>

#include "common/datatypes.h"
#include "common/schema.h"

namespace tajo {

class Schema;

class Column;

class VecRowBlock;

class Exec {

  enum ExecState {
    NEW,
    INITED,
    CLOSED
  };

 public:
  Exec() : state(NEW) {
  };

  virtual ~Exec() {
  };

  inline ExecState GetState() const {
    return this->state;
  }

  inline void SetState(ExecState state) {
    this->state = state;
  }

  inline void Init() {
    CHECK(GetState() == NEW);
    DoInit();
    SetState(INITED);
  }

  inline bool GetNext(VecRowBlock *vrows) {
    DCHECK(GetState() == INITED);
    return DoGetNext(vrows);
  }

  inline void Reset() {
    CHECK(GetState() != CLOSED);
    DoReset();
  }

  inline void Close() {
    DoClose();
    SetState(CLOSED);
  }

  virtual void SetTargets(const std::vector<Column> &targets) = 0;

  virtual inline const Schema &schema() const = 0;

 private:

  virtual void DoInit() = 0;

  virtual bool DoGetNext(VecRowBlock *vrows) = 0;

  virtual void DoReset() = 0;

  virtual void DoClose() = 0;

  ExecState state;
};

}


#endif /* TAJO_STORAGE_SCANNER_H_ */

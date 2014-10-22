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

#ifndef TAJO_VECTOR_VECTORS_H_
#define TAJO_VECTOR_VECTORS_H_

#include <math.h>
#include "util/memory.h"
#include "common/datatypes.h"
#include "common/stringpiece.h"

namespace tajo {

class Schema;

typedef void *Vector;

// Memory Structure of Vectorized Row Block (designed and implemented by hyunsik)
//
// Main design goals:
//  * Using consolidated memory spaces as many as possible, reducing cache and TLB misses
//  * Preserving cache locality which placing data in adjacent memory
//  * Avoiding memory copies
//  * Eliminating memory allocations overheads and reusing allocated memory
//  * Placing data on aligned memory spaces allowing direct SIMD operations

class VecRowBlock {

 public:
  VecRowBlock(const Schema &schema, size_t vector_size, const BufferAllocator *allocator);

  VecRowBlock(const VecRowBlock &copy);

  ~VecRowBlock();

  inline const uint32_t VectorSize() {
    return vector_size_;
  }

  inline const uint32_t selected_num() {
    return selected_num_;
  }

  inline void SetSelectedNum(uint32_t n) {
    this->selected_num_ = n;
  }

  template<class T>
  inline T *GetVector(int idx) const {
    return reinterpret_cast<T *>(vectors_[idx]);
  }

  inline void SetNullBit(const int columnIdx, const int rowIdx) {
    SetToBitVec(nullVecs_[columnIdx], rowIdx);
  }

  inline void UnsetNullBit(const int columnIdx, const int rowIdx) {
    UnsetToBitVec(nullVecs_[columnIdx], rowIdx);
  }

  inline bool IsNotNull(const int colIdx, const int rowIdx) {
    return GetFromBitVec(nullVecs_[colIdx], rowIdx);
  }

  void PutBool(const int colIdx, const int rowIdx, const bool val);

  inline bool GetBool(const int colIdx, const int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == BOOLEAN);

    return GetFromBitVec(vectors_[colIdx], rowIdx);
  }

  inline void PutInt1(const int colIdx, const int rowIdx, const tajo_int1_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT1);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_int1_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_int1_t GetInt1(const int colIdx, const int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT1);

    return reinterpret_cast<tajo_int1_t *>(vectors_[colIdx])[rowIdx];
  }

  inline void PutInt2(int colIdx, int rowIdx, tajo_int2_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT2);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_int2_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_int2_t GetInt2(int colIdx, int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT2);

    return reinterpret_cast<tajo_int2_t *>(vectors_[colIdx])[rowIdx];
  }

  inline void PutInt4(const int colIdx, const int rowIdx, tajo_int4_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT4);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_int4_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_int4_t GetInt4(int colIdx, int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT4);

    return reinterpret_cast<tajo_int4_t *>(vectors_[colIdx])[rowIdx];
  }

  inline void PutInt8(int colIdx, int rowIdx, tajo_int8_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT8);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_int8_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_int8_t GetInt8(int colIdx, int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == INT8);

    return reinterpret_cast<tajo_int8_t *>(vectors_[colIdx])[rowIdx];
  }

  inline void PutFloat4(int colIdx, int rowIdx, tajo_float4_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == FLOAT4);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_float4_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_float4_t GetFloat4(int colIdx, int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == FLOAT4);

    return reinterpret_cast<tajo_float4_t *>(vectors_[colIdx])[rowIdx];
  }

  inline void PutFloat8(int colIdx, int rowIdx, tajo_float8_t val) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == FLOAT8);

    SetNullBit(colIdx, rowIdx);
    reinterpret_cast<tajo_float8_t *>(vectors_[colIdx])[rowIdx] = val;
  }

  inline tajo_float8_t GetFloat8(int colIdx, int rowIdx) {
    DCHECK_LE(colIdx, column_num_);
    DCHECK_LE(rowIdx, vector_size_);
    DCHECK(types[colIdx] == FLOAT8);

    return reinterpret_cast<tajo_float8_t *>(vectors_[colIdx])[rowIdx];
  }

  void PutChars(const int colIdx, const int rowIdx, const char *val);

  void PutChars(const int colIdx, const int rowIdx, std::string val);

  inline void PutChars(const int colIdx, const int rowIdx, StringPiece &val) {
    PutChars(colIdx, rowIdx, val.data());
  }

  const StringPiece GetChars(const int colIdx, const int rowIdx);

  inline void PutVarChar(const int colIdx, const int rowIdx, const char *val, const uint32_t len) {
    PutText(colIdx, rowIdx, val, std::min(maxLengths[colIdx], len));
  }

  void PutVarChar(const int colIdx, const int rowIdx, std::string val);

  inline void PutVarChar(const int colIdx, const int rowIdx, StringPiece &val) {
    PutVarChar(colIdx, rowIdx, val.data(), val.length());
  }

  inline const StringPiece GetVarChar(const int colIdx, const int rowIdx) {
    return GetText(colIdx, rowIdx);
  }

  void PutText(const int colIdx, const int rowIdx, const char *val, const uint32_t len);

  void PutText(const int colIdx, const int rowIdx, std::string val);

  inline void PutText(const int colIdx, const int rowIdx, StringPiece &val) {
    PutText(colIdx, rowIdx, val.data(), val.length());
  }

  const StringPiece GetText(const int colIdx, const int rowIdx);

 private:
  static const int32_t DEFAULT_PAGE_SIZE = 1024;

  const Schema &schema_;
  const uint32_t column_num_;
  const uint32_t vector_size_;
  uint32_t selected_num_;
  BufferAllocator *allocator_;
  Type *types;
  uint32_t *maxLengths;
  vector<Vector> vectors_;

  // for fixed memory areas
  Buffer *fixedArea_;
  size_t fixedAreaMemorySize_;
  Vector *nullVecs_;

  // for variable areas
  bool need_variable_area_;
  Buffer **currentPageAddrs_;
  char **nextPtr_;
  vector<Buffer *> *pageRefs_;
  size_t variableMemSize_;

  // for null header initialization
  size_t ComputeNullHeaderSizePerColumn() const;

  size_t ComputeNullHeaderSize() const;

  void InitMemory(const bool variableAreaNeeded);

  void ClearFixedArea();

  void InitVariableArea();

  Buffer *CreatePage();

  // for bit vector
  void SetToBitVec(void *bitVec, int rowIdx);

  void UnsetToBitVec(Vector bitVec, int rowIdx);

  int GetFromBitVec(void *bitVec, int rowIdx);

  inline char *GetFixedElementPtr(const uint32_t colIdx, const uint32_t rowIdx, size_t elemSize) const {
    return static_cast<char *>(vectors_[colIdx]) + (rowIdx * elemSize);
  }
};

}

#endif // TAJO_VECTOR_VECTORS_H_

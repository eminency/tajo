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

#include<iostream>

#include <algorithm>
#include "common/schema.h"
#include "vectors.h"

// Fixed Area consists of two areas: one is null header area and fixed column value area.

using namespace std;

namespace tajo {

VecRowBlock::VecRowBlock(Schema const &schema, size_t vector_size, const BufferAllocator *allocator)
    : schema_(schema),
      column_num_(schema.size()),
      vector_size_(vector_size) {

  allocator_ = const_cast<BufferAllocator *>(allocator);

  bool needVariableArea = false;

  types = new Type[column_num_];
  maxLengths = new uint32_t[column_num_];
  for (int i = 0; i < column_num_; i++) {
    Column c = schema_.column(i);
    needVariableArea |= isFixedDataType(c.datatype());
    types[i] = c.datatype().type();
    if (c.datatype().has_length()) {
      maxLengths[i] = c.datatype().length();
    }
  }

  need_variable_area_ = needVariableArea;

  if (need_variable_area_) {
    currentPageAddrs_ = new Buffer *[column_num_];
    nextPtr_ = new char *[column_num_];
    pageRefs_ = new vector<Buffer *>;
  }

  InitMemory(needVariableArea);
}

VecRowBlock::VecRowBlock(const VecRowBlock &that)
    : schema_(that.schema_),
      column_num_(that.column_num_),
      vector_size_(that.column_num_),
      types(that.types),
      vectors_(that.vectors_),
      currentPageAddrs_(that.currentPageAddrs_),
      nextPtr_(that.nextPtr_),
      pageRefs_(that.pageRefs_) {
}

VecRowBlock::~VecRowBlock() {
  delete[] types;
  delete[] nullVecs_;
  delete fixedArea_;

  if (need_variable_area_) {
    delete[] currentPageAddrs_;
    delete[] nextPtr_;
    delete pageRefs_;
  }
}

void VecRowBlock::InitMemory(const bool variableAreaNeeded) {
  Type types[column_num_];
  size_t maxLengths[column_num_];
  for (int i = 0; i < column_num_; i++) {
    Column c = schema_.column(i);
    types[i] = c.datatype().type();
    maxLengths[i] = c.datatype().length();
  }

  size_t totalSize = 0;
  size_t eachNullHeaderBytes = ComputeNullHeaderSizePerColumn();
  size_t totalNullHeaderBytes = ComputeNullHeaderSize();
  totalSize += totalNullHeaderBytes;

  ////////////////////////////////////////
  // Fixed Area Initialization Begin
  ////////////////////////////////////////

  // getting fixed area size
  for (int i = 0; i < column_num_; i++) {
    Column column = schema_.column(i);
    totalSize += ComputeAlignedSize(SizeOfVectorElement(column.datatype()) * vector_size_);
  }
  fixedAreaMemorySize_ = totalSize;
  fixedArea_ = allocator_->Allocate(fixedAreaMemorySize_);

  nullVecs_ = new Vector[column_num_];
  // initialize null headers
  for (int i = 0; i < column_num_; i++) {
    if (i == 0) {
      nullVecs_[0] = fixedArea_->data();
    } else {
      nullVecs_[i] = static_cast<unsigned char *>(nullVecs_[i - 1]) + eachNullHeaderBytes;
      DCHECK((unsigned char *)nullVecs_[i] - (unsigned char *)nullVecs_[i-1]
          == ComputeAlignedSize((long)ceil(static_cast<double>(vector_size_)) / 8));
    }
  }

  long perVecSize;
  for (int i = 0; i < column_num_; i++) {
    if (i == 0) {
      vectors_.push_back(static_cast<unsigned char *>(fixedArea_->data()) + totalNullHeaderBytes);
    } else {
      const Column prevColumn = schema_.column(i - 1);
      perVecSize = ComputeAlignedSize(SizeOfVectorElement(prevColumn.datatype()) * vector_size_);
      vectors_.push_back(static_cast<unsigned char *>(vectors_[i - 1]) + perVecSize);
    }
  }

  ClearFixedArea();
  ////////////////////////////////////////
  // Fixed Area Initialization End
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Variable Area Initialization Begin
  ////////////////////////////////////////
  if (variableAreaNeeded) {
    InitVariableArea();
  }
  ////////////////////////////////////////
  // Variable Area Initialization End
  ////////////////////////////////////////
}

void VecRowBlock::ClearFixedArea() {
  // initialize null headers
  memset(nullVecs_[0], 0, ComputeNullHeaderSize());

  // initializes some fixed type vector(s) which have a default value.
  for (int i = 0; i < column_num_; i++) {
    const Column c = schema_.column(i);
    size_t eachVectorSize = ComputeAlignedSize(SizeOfVectorElement(c.datatype()) * vector_size_);
    switch (types[i]) {
      case BOOLEAN:
      case CHAR:
        memset(vectors_[i], 0x00, eachVectorSize);
        break;
      default:
        break;
    }
  }
}

void VecRowBlock::InitVariableArea() {
  for (int i = 0; i < column_num_; i++) {
    Column c = schema_.column(i);
    if (isFixedDataType(c.datatype())) {
      currentPageAddrs_[i] = NULL;
      nextPtr_[i] = NULL;
    } else {
      currentPageAddrs_[i] = CreatePage();
      nextPtr_[i] = static_cast<char *>(currentPageAddrs_[i]->data());
    }
  }
}


// [next address, 0x0 last page. Otherwise, has next page (8 bytes) ] [PAGE payload (4096 by default) ]
Buffer *VecRowBlock::CreatePage() {
  Buffer *page = allocator_->Allocate(DEFAULT_PAGE_SIZE);
  pageRefs_->push_back(page);
  variableMemSize_ += DEFAULT_PAGE_SIZE;
  return page;
}

inline size_t VecRowBlock::ComputeNullHeaderSizePerColumn() const {
  return ComputeAlignedSize((long) ceil(static_cast<double>(vector_size_) / 8));
}

inline size_t VecRowBlock::ComputeNullHeaderSize() const {
  return ComputeNullHeaderSizePerColumn() * column_num_;
}

void VecRowBlock::SetToBitVec(void *bitVec, int rowIdx) {
  int chunkId = rowIdx / 8;
  int offset = rowIdx & 7; // == rowIdx % 8
  unsigned char *address = static_cast<unsigned char *>(bitVec) + chunkId;
  *address = *address | (0x01 << offset);
}

void VecRowBlock::UnsetToBitVec(Vector bitVec, int rowIdx) {
  int chunkId = rowIdx / 8;
  long offset = rowIdx & 7;
  unsigned char *address = static_cast<unsigned char *>(bitVec) + chunkId;
  *address = *address ^ (0x01 << offset);
}

int VecRowBlock::GetFromBitVec(void *bitVec, int rowIdx) {
  int chunkId = rowIdx / 8;
  long offset = rowIdx & 7; // == rowIdx & 8
  unsigned char *address = static_cast<unsigned char *>(bitVec) + chunkId;
  return (*address >> offset) & 0x01;
}

void VecRowBlock::PutBool(const int colIdx, const int rowIdx, const bool val) {
  DCHECK_LE(colIdx, column_num_);
  DCHECK_LE(rowIdx, vector_size_);

  if (val) {
    SetNullBit(colIdx, rowIdx);
  } else {
    UnsetNullBit(colIdx, rowIdx);
  }
  SetToBitVec(vectors_[colIdx], rowIdx);
}

void VecRowBlock::PutChars(const int colIdx, const int rowIdx, const char *val) {
  DCHECK_LE(colIdx, column_num_);
  DCHECK_LE(rowIdx, vector_size_);
  DCHECK_EQ(types[colIdx], CHAR);

  SetNullBit(colIdx, rowIdx);
  memcpy(GetFixedElementPtr(colIdx, rowIdx, maxLengths[colIdx]), val, maxLengths[colIdx]);
}

const StringPiece VecRowBlock::GetChars(const int colIdx, const int rowIdx) {
  return StringPiece(GetFixedElementPtr(colIdx, rowIdx, maxLengths[colIdx]), maxLengths[colIdx]);
}

const StringPiece VecRowBlock::GetText(const int colIdx, const int rowIdx) {
  DCHECK_LE(colIdx, column_num_);
  DCHECK_LE(rowIdx, vector_size_);
  DCHECK(types[colIdx] == VARCHAR || types[colIdx] == TEXT);

  uint32_t len;
  char *varchar;

  void *elementPtr = GetFixedElementPtr(colIdx, rowIdx, sizeof(void *));
  memcpy(&varchar, elementPtr, sizeof(void *));

  memcpy(&len, varchar, 4);
  varchar += 4;
  return StringPiece(varchar, len);
}

void VecRowBlock::PutText(const int colIdx, const int rowIdx, const char *val, const uint32_t len) {
  DCHECK_LE(colIdx, column_num_);
  DCHECK_LE(rowIdx, vector_size_);
  DCHECK(types[colIdx] == VARCHAR || types[colIdx] == TEXT);

  SetNullBit(colIdx, rowIdx);

  char *currentPage = static_cast<char *>(currentPageAddrs_[colIdx]->data());
  char *nextPtr = nextPtr_[colIdx];
  uint32_t usedMemory = nextPtr - currentPage;

  if ((DEFAULT_PAGE_SIZE - usedMemory) < (len + 4)) { // create newly page
    Buffer *newBuf = CreatePage();
    currentPageAddrs_[colIdx] = newBuf;
    currentPage = static_cast<char *>(newBuf->data());
    nextPtr = currentPage;
  }

  void *elementPtr = GetFixedElementPtr(colIdx, rowIdx, sizeof(void *));
  memcpy(elementPtr, &nextPtr, sizeof(void *));

  // put length into
  memcpy(nextPtr, &len, 4);

  nextPtr += 4;

  // copy string
  memcpy(nextPtr, val, len);
  nextPtr += len;

  nextPtr_[colIdx] = nextPtr;
  // TODO - move to aligned memory
}

} // namespace tajo

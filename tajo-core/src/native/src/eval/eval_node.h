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


#ifndef TAJO_EVAL_EVAL_NODE_H_
#define TAJO_EVAL_EVAL_NODE_H_

#include <common/logging.h>
#include <DataTypes.pb.h>

namespace tajo {

enum EvalType {
  // Unary expression
  NOT,

  // Logical, Comparison, and Arithmetic
  AND,
  OR,
  EQUAL,
  IS_NULL,
  NOT_EQUAL,
  LTH,
  LEQ,
  GTH,
  GEQ,
  PLUS,
  MINUS,
  MODULAR,
  MULTIPLY,
  DIVIDE,

  // Binary Bitwise expressions
  BIT_AND,
  BIT_OR,
  BIT_XOR,

  // Function
  WINDOW_FUNCTION,
  AGG_FUNCTION,
  FUNCTION,

  // String operator or pattern matching predicates
  LIKE,
  SIMILAR_TO,
  REGEX,
  CONCATENATE,

  // Other predicates
  BETWEEN,
  CASE,
  IF_THEN,
  IN,

  // Value or Reference
  SIGNED,
  CAST,
  ROW_CONSTANT,
  FIELD,
  CONST,

  UNKNOWN_EVAL_TYPE
};

class EvalNode {

 public:
  EvalNode(EvalType type) : type_(type) {
  }

  virtual ~EvalNode() = 0;

  EvalType type() {
    return type_;
  }

  static bool isComparison(EvalType &type) {
    bool match = false;

    match |= type == EQUAL;
    match |= type == NOT_EQUAL;
    match |= type == LTH;
    match |= type == LEQ;
    match |= type == GTH;
    match |= type == GEQ;
    match |= type == BETWEEN;

    return match;
  }

  static bool IsArithmetic(EvalType &type) {
    bool match = false;
    match |= type == PLUS;
    match |= type == MINUS;
    match |= type == MULTIPLY;
    match |= type == DIVIDE;
    match |= type == MODULAR;
    return match;
  }

 protected:
  EvalType type_;
  DataType* data_type_;

  void SetValueType(DataType* data_type) {
    this->data_type_ = data_type;
  }

 private:
};

class UnaryEval : public EvalNode {

 public:
  UnaryEval(EvalType type, EvalNode* child) : EvalNode(type), child_(child) {
    CHECK_NOTNULL(child);
  }
  virtual ~UnaryEval() = 0;

  EvalNode* child() const {
    return this->child_;
  }

 protected:

 private:
   EvalNode* child_;
};

class BinaryEval : public EvalNode {

 public:
  BinaryEval(EvalType type, EvalNode* lhs, EvalNode* rhs)
      : EvalNode(type),
        lhs_(lhs),
        rhs_(rhs) {
    CHECK_NOTNULL(lhs);
    CHECK_NOTNULL(rhs);
  }

  virtual ~BinaryEval() = 0;

  EvalNode* lhs() {
    return lhs_;
  }

  EvalNode* rhs() {
    return rhs_;
  }

 protected:
  EvalNode* lhs_;
  EvalNode* rhs_;

 private:
};

} // namespace tajo

#endif // TAJO_EVAL_EVAL_NODE_H_

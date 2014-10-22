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

#ifndef TAJO_PLAN_LOGICAL_NODE_H_
#define TAJO_PLAN_LOGICAL_NODE_H_

namespace tajo {

class Schema;

enum NodeType {
  EXPRS,
  PROJECTION,
  LIMIT,
  SORT,
  HAVING,
  GROUP_BY,
  WINDOW_AGG,
  SELECTION,
  JOIN,
  UNION,
  EXCEPT,
  INTERSECT,
  TABLE_SUBQUERY,
  SCAN,
  PARTITIONS_SCAN,
  BST_INDEX_SCAN,
  STORE,
  INSERT,
  DISTINCT_GROUP_BY,
  ROOT,
  EXPLAIN,

  CREATE_DATABASE,
  DROP_DATABASE,
  CREATE_TABLE,
  DROP_TABLE,
  ALTER_TABLESPACE,
  ALTER_TABLE,
  TRUNCATE_TABLE,

  UNKNOWN_NODE_TYPE
};

class LogicalNode {

 public:
  LogicalNode(NodeType type) : type_(type) {
  }

  virtual ~LogicalNode() = 0;

  inline NodeType type() const {
    return type_;
  }

  Schema* input_schema() const {
    return this->in_schema_;
  }

  Schema* out_schema() const {
    return this->out_schema_;
  }

 protected:
  const NodeType type_;
  Schema* in_schema_;
  Schema* out_schema_;
};

class UnaryNode : LogicalNode {

 public:
  UnaryNode(NodeType type, LogicalNode* child)
   : LogicalNode(type),
     child_(child) {
  }

  virtual ~UnaryNode() = 0;

  LogicalNode* child() const {
    return child_;
  }


 protected:
  LogicalNode* child_;
};

class BinaryNode : LogicalNode {

 public:
  BinaryNode(NodeType type, LogicalNode* left, LogicalNode* right)
   : LogicalNode(type),
     left_(left),
     right_(right) {
  }

  virtual ~BinaryNode();

  LogicalNode* left() const {
    return this->left_;
  }

  LogicalNode* right() const {
    return this->right_;
  }

 protected:
  LogicalNode* left_;
  LogicalNode* right_;
};

} /* namespace tajo */

#endif /* TAJO_PLAN_LOGICAL_NODE_H_ */

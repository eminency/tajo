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

#include "logical_plan_builder.h"
#include <json_spirit.h>
#include <glog/logging.h>
#include <eval/eval_node.h>

namespace tajo {

using namespace std;
using namespace json_spirit;

LogicalNode* LogicalPlanBuilder::BuildPlan(const std::string& json) {

  Value value;
  json_spirit::read(json, value);

  DeserializeObject(value);

  return nullptr;
}

LogicalNode* LogicalPlanBuilder::DeserializeObject(const Value& data) {
  const Object obj = data.get_obj();

  std::string type;
  const Object* body;

  for (int i = 0; i < obj.size(); i++) {
    const Pair& pair = obj[i];
    const std::string& name  = pair.name_;
    const Value&  data  = pair.value_;

    if (name == "type") {
      type = data.get_str();
    } else if (name == "body") {
      body = &data.get_obj();
    }
  }

  NodeType node_type = GetNodeType(type);

  return nullptr;
}

NodeType LogicalPlanBuilder::GetNodeType(const string& type) {
  if (type == "BST_INDEX_SCAN") return BST_INDEX_SCAN;
  else if (type == "CREATE_TABLE") return CREATE_TABLE;
  else if (type == "DROP_TABLE") return DROP_TABLE;
  else if (type == "EXCEPT")    return EXCEPT;
  else if (type == "EXPLAIN")   return EXPLAIN;
  else if (type == "EXPRS")     return EXPRS;
  else if (type == "GROUP_BY")  return GROUP_BY;
  else if (type == "INTERSECT") return INTERSECT;
  else if (type == "LIMIT")     return LIMIT;
  else if (type == "JOIN")      return JOIN;
  else if (type == "PROJECTION") return PROJECTION;
  else if (type == "ROOT")      return ROOT;
  else if (type == "SCAN")      return SCAN;
  else if (type == "SELECTION") return SELECTION;
  else if (type == "STORE")     return STORE;
  else if (type == "SORT")      return SORT;
  else if (type == "UNION")     return UNION;
  else {
    CHECK(false) << "Unknown NodeType: " << type;
    return UNKNOWN_NODE_TYPE;
  }
}

EvalType LogicalPlanBuilder::GetEvalType(const string& type) {
  if (type == "NOT") return NOT;
  else if (type == "AND") return AND;
  else if (type == "OR") return OR;
  else if (type == "EQUAL") return EQUAL;
  else if (type == "IS_NULL") return IS_NULL;
  else if (type == "NOT_EQUAL") return NOT_EQUAL;
  else if (type == "LTH") return LTH;
  else if (type == "LEQ") return LEQ;
  else if (type == "GTH") return GTH;
  else if (type == "GEQ") return GEQ;
  else if (type == "PLUS") return PLUS;
  else if (type == "MINUS") return MINUS;
  else if (type == "MODULAR") return MODULAR;
  else if (type == "MULTIPLY") return MULTIPLY;
  else if (type == "DIVIDE") return DIVIDE;
  else if (type == "AGG_FUNCTION") return AGG_FUNCTION;
  else if (type == "FUNCTION") return FUNCTION;
  else if (type == "LIKE") return LIKE;
  else if (type == "SIMILAR_TO") return SIMILAR_TO;
  else if (type == "REGEX") return REGEX;
  else if (type == "CONCATENATE") return CONCATENATE;
  else if (type == "BETWEEN") return BETWEEN;
  else if (type == "CASE") return CASE;
  else if (type == "IF_THEN") return IF_THEN;
  else if (type == "IN") return IN;
  else if (type == "CAST") return CAST;
  else if (type == "ROW_CONSTANT") return ROW_CONSTANT;
  else if (type == "FIELD") return FIELD;
  else if (type == "CONST") return CONST;
  else {
    CHECK(false) << "Unknown EvalType: " << type;
    return UNKNOWN_EVAL_TYPE;
  }
}

} // namespace tajo

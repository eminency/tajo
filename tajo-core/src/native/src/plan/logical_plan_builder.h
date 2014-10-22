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


#ifndef TAJO_PLAN_LOGICAL_PLAN_BUILDER_H_
#define TAJO_PLAN_LOGICAL_PLAN_BUILDER_H_

#include <string>
#include <json_spirit.h>

#include "logical_node.h"
#include "eval/eval_node.h"

namespace tajo {

using namespace std;
using namespace json_spirit;

class LogicalPlanBuilder {
 public:

  static LogicalNode* BuildPlan(const std::string& json);
  static LogicalNode* DeserializeObject(const Value& value);

  static NodeType GetNodeType(const string& type);
  static EvalType GetEvalType(const string& type);

 private:

};

} // namespace tajo

#endif // TAJO_PLAN_LOGICAL_PLAN_BUILDER_H_

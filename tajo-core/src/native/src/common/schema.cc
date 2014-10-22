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

#include <iostream>
#include "datatypes.h"
#include "schema.h"

using namespace std;

namespace tajo {

Column::Column() {

}

Column::Column(const Column &src) :
    qualified_name_(src.qualified_name_),
    proto_(src.proto_) {

}

Column::Column(const ColumnProto &column) :
    proto_(column) {
  if (!column.qualifier().empty()) {
    std::stringstream ss;
    ss << column.qualifier() << "."
        << column.columnname();
    qualified_name_ = ss.str();
  } else
    qualified_name_ = column.columnname();
}

const string Schema::NAME_UNKNOWN = "_UNKNOWN";

Schema::Schema() {
}

Schema::Schema(const Schema &schema) :
    proto_(schema.proto_) {

  Build();
}

Schema::Schema(const SchemaProto &proto) :
    proto_(proto) {

  Build();
}

Schema::~Schema() {

}

void Schema::Build() {
  for (int i = 0; i < proto_.fields_size(); ++i) {
    Column column(proto_.fields(i));
    SetNameIndex(column.qualified_name(), i, column);
  }
}


void Schema::AddColumn(const Column &column) {
  ColumnProto *new_column = proto_.add_fields();

  new_column->CopyFrom(column.proto());
}


void Schema::AddColumn(const std::string &name, Type type, int length) {

  ColumnProto *column = proto_.add_fields();
  int pos = name.find('.');
  if (pos > 0) {
    column->set_qualifier(name.substr(0, pos));
    column->set_columnname(name.substr(pos + 1));
  } else
    column->set_columnname(name);
  DataType *dataType = column->mutable_datatype();
  dataType->set_type(type);

  if (isRequiredLength(*dataType)) {
    dataType->set_length(length);
  }
}

void Schema::SetNameIndex(const std::string &name, int idx, const Column &column) {
  CHECK(column_map_by_name_.find(name) == column_map_by_name_.end());
  DCHECK_EQ(idx, columns_.size());
  column_map_by_name_[name] = column;
  idx_map_by_name_[name] = idx;
  name_map_by_idx_[idx] = name;

  columns_.push_back(column);
}

const Column &Schema::GetColumn(const std::string &name) {
  return column_map_by_name_[name];
}

int Schema::GetColumnId(const std::string &name) const {
  map<string, int>::const_iterator it = idx_map_by_name_.find(name);
  if (it != idx_map_by_name_.end()) return it->second;
  return -1;
}

const string &Schema::GetColumnName(int idx) const {
  map<int, string>::const_iterator it = name_map_by_idx_.find(idx);
  if (it != name_map_by_idx_.end()) return it->second;

  return NAME_UNKNOWN;
}


} /* namespace tajo */

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

#ifndef TAJO_COMMON_SCHEMA_HPP_
#define TAJO_COMMON_SCHEMA_HPP_

#include <stddef.h>
#include <DataTypes.pb.h>
#include "common/logging.h"
#include "common/types-inl.h"

namespace tajo {
class Column;

class SchemaProto;

class ColumnProto;

class Column {
 public:
  Column();

  Column(const Column &src);

  Column(const ColumnProto &column);

  Column &operator=(const Column &src) {
    proto_.CopyFrom(src.proto_);
    qualified_name_ = src.qualified_name_;
    return *this;
  }

  const DataType &datatype() const {
    return proto_.datatype();
  }

  const std::string &qualified_name() const {
    return qualified_name_;
  }

  void SetDateType(Type type) {
    proto_.mutable_datatype()->set_type(type);
  }

  const ColumnProto &proto() const {
    return proto_;
  }

 private:
  std::string qualified_name_;
  ColumnProto proto_;
};

class Schema {
 public:
  Schema();

  Schema(const Schema &schema);

  Schema(const SchemaProto &proto);

  virtual ~Schema();

  void AddColumn(const Column &column);

  void AddColumn(const std::string &name, tajo::Type type, int length = 1);

  void Build();

  const Column &GetColumn(const std::string &name);

  int GetColumnId(const std::string &name) const;

  const std::string &GetColumnName(int idx) const;

  int size() const {
    return proto_.fields_size();
  };

  const SchemaProto &proto() const {
    return proto_;
  }

  const std::vector<Column> &columns() const {
    return columns_;
  }

  const Column &column(int idx) const {
    return columns_[idx];
  }

  Type type(int idx) const {
    return columns_[idx].datatype().type();
  }

 private:
  static const std::string NAME_UNKNOWN;

  SchemaProto proto_;
  std::map<std::string, Column> column_map_by_name_;
  std::map<std::string, int> idx_map_by_name_;
  std::map<int, std::string> name_map_by_idx_;
  std::vector<Column> columns_;

  void SetNameIndex(const std::string &name, int idx, const Column &column);

};

} /* namespace tajo */
#endif /* TAJO_COMMON_SCHEMA_HPP_ */

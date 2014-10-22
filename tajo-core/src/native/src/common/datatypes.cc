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

#include <glog/logging.h>
#include "common/datatypes.h"

namespace tajo {

bool isRequiredLength(const DataType &dataType) {
  bool canHaveLength = false;

  Type type = dataType.type();

  canHaveLength |= type == CHAR;
  canHaveLength |= type == VARCHAR;
  canHaveLength |= type == VARBINARY;

  return canHaveLength;
}

bool isFixedDataType(const DataType &dataType) {
  bool fixedType = false;
  Type type = dataType.type();

  fixedType |= type == CHAR;
  fixedType |= type == BOOLEAN;
  fixedType |= type == INT1;
  fixedType |= type == INT2;
  fixedType |= type == INT4;
  fixedType |= type == INT8;
  fixedType |= type == FLOAT4;
  fixedType |= type == FLOAT8;
  fixedType |= type == INET4;
  fixedType |= type == TIMESTAMP;
  fixedType |= type == DATE;
  fixedType |= type == TIME;

  return fixedType;
}

size_t SizeOfVectorElement(const DataType & dataType) {
  switch (dataType.type()) {
    case CHAR:
      return (dataType.has_length() ? dataType.length() : 1);
    case BOOLEAN:
      return SIZE_OF_BOOL;
    case INT1:
      return SIZE_OF_INT1;
    case INT2:
      return SIZE_OF_INT2;
    case INT4:
      return SIZE_OF_INT4;
    case INT8:
      return SIZE_OF_INT8;
    case FLOAT4:
      return SIZE_OF_FLOAT4;
    case FLOAT8:
      return SIZE_OF_FLOAT8;
    case TIMESTAMP:
      return SIZE_OF_TIMESTAMP;
    case DATE:
      return SIZE_OF_DATE;
    case TIME:
      return SIZE_OF_TIME;
    case INTERVAL:
      return SIZE_OF_INTERVAL;
    case VARCHAR:
      return 8;
    case TEXT:
      return 8;
    default:
      CHECK(false) << "Unexpected data type: " << dataType.type();
      return -1;
  }
}

} // namespace tajo

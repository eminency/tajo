#ifndef TAJO_COMMON_DATATYPES_
#define TAJO_COMMON_DATATYPES_

#include "DataTypes.pb.h"
#include "CatalogProtos.pb.h"

namespace tajo {
  typedef bool tajo_bool_t;
  typedef int8_t tajo_int1_t;
  typedef int16_t tajo_int2_t;
  typedef int32_t tajo_int4_t;
  typedef int64_t tajo_int8_t;
  typedef float tajo_float4_t;
  typedef double tajo_float8_t;
  typedef int64_t tajo_timestamp_t;
  typedef int32_t tajo_date_t;
  typedef int64_t tajo_time_t;
  typedef uintptr_t tajo_interval_t;

  const uint32_t SIZE_OF_BOOL = sizeof(bool);
  const uint32_t SIZE_OF_INT1 = sizeof(int8_t);
  const uint32_t SIZE_OF_INT2 = sizeof(int16_t);
  const uint32_t SIZE_OF_INT4 = sizeof(int32_t);
  const uint32_t SIZE_OF_INT8 = sizeof(int64_t);
  const uint32_t SIZE_OF_FLOAT4 = sizeof(float);
  const uint32_t SIZE_OF_FLOAT8 = sizeof(double);
  const uint32_t SIZE_OF_INET4 = sizeof(int32_t);
  const uint32_t SIZE_OF_TIMESTAMP = sizeof(int64_t);
  const uint32_t SIZE_OF_DATE = sizeof(int32_t);
  const uint32_t SIZE_OF_TIME = sizeof(int64_t);
  const uint32_t SIZE_OF_INTERVAL = 12;

  bool isRequiredLength(const DataType &dataType);

  bool isFixedDataType(const DataType &dataType);

  size_t SizeOfVectorElement(const DataType & dataType);

} // namespace tajo

#endif // TAJO_COMMON_DATATYPES_

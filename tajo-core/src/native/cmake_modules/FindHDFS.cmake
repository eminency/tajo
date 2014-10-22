set(HDFS_SEARCH_HEADER_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/include
)

set(HDFS_SEARCH_LIB_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/lib
)

find_path(HDFS_INCLUDE_DIR hdfs.h PATHS
  ${HDFS_SEARCH_HEADER_PATH}
  $ENV{HADOOP_HOME}/include
  # make sure we don't accidentally pick up a different version
  NO_DEFAULT_PATH
)

find_library(HDFS_LIBS NAMES hdfs 
  PATHS 
  ${HDFS_SEARCH_LIB_PATH}
  $ENV{HADOOP_HOME}/lib/native
)

message(STATUS "HDFS_LIBS: ${HDFS_LIBS}")

if (HDFS_LIBS)
  set(HDFS_FOUND TRUE)
  set(HDFS_LIB ${HDFS_LIBS})
  set(HDFS_SHARED_LIB ${HDFS_LIB})
else ()
  set(HDFS_FOUND FALSE)
endif ()

if (HDFS_FOUND)
  if (NOT HDFS_FIND_QUIETLY)
    message(STATUS "Found hdfs headers: ${HDFS_INCLUDE_DIR}")
    message(STATUS "Found hdfs library: ${HDFS_LIB}")
  endif ()
else ()
  message(STATUS "hdfs includes and libraries NOT found. "
    "Looked for headers in ${HDFS_SEARCH_HEADER_PATH}, "
    "and for libs in ${HDFS_SEARCH_LIB_PATH}, ")
endif ()

mark_as_advanced(
  HDFS_INCLUDE_DIR
  HDFS_LIBS
  HDFS_SHARED_LIB
)

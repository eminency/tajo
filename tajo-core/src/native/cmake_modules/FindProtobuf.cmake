# This module defines
#  PROTOBUF_INCLUDE_DIR, directory containing protobuf headers
#  PROTOBUF_LIBS, directory containing protobuf libraries
#  PROTOBUF_STATIC_LIB, path to libprotobuf.a
#  PROTOBUF_FOUND, whether protobuf has been found
#  PROTOBUF_PROTOC_EXECUTABLE, protoc file path

set(PROTOBUF_SEARCH_HEADER_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/include/google/protobuf
)

set(PROTOBUF_SEARCH_LIB_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/lib
)

set(PROTOBUF_SEARCH_BIN_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/bin
)

find_path(PROTOBUF_INCLUDE_DIR message.h PATHS
  ${PROTOBUF_SEARCH_HEADER_PATH}
  # make sure we don't accidentally pick up a different version
  NO_DEFAULT_PATH
)

find_library(PROTOBUF_LIBS NAMES protobuf PATHS ${PROTOBUF_SEARCH_LIB_PATH} NO_DEFAULT_PATH)

find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc PATHS ${PROTOBUF_SEARCH_BIN_PATH} NO_DEFAULT_PATH)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(protobuf DEFAULT_MSG PROTOBUF_INCLUDE_DIR PROTOBUF_LIBS PROTOBUF_PROTOC_EXECUTABLE)

if (PROTOBUF_INCLUDE_DIR AND PROTOBUF_LIBS AND PROTOBUF_PROTOC_EXECUTABLE)
  set(PROTOBUF_FOUND TRUE)
  set(PROTOBUF_STATIC_LIB ${PROTOBUF_SEARCH_LIB_PATH}/libprotobuf.a)
else ()
  set(PROTOBUF_FOUND FALSE)
endif ()

if (PROTOBUF_FOUND)
  if (NOT PROTOBUF_FIND_QUIETLY)
    message(STATUS "Found protobuf headers: ${PROTOBUF_INCLUDE_DIR}")
    message(STATUS "Found protobuf library: ${PROTOBUF_LIBS}")
    message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
  endif ()
else ()
  message(STATUS "protobuf includes and libraries and protoc NOT found. "
    "Looked for headers in ${PROTOBUF_SEARCH_HEADER_PATHS}, "
    "and for libs in ${PROTOBUF_SEARCH_LIB_PATH}, ")
endif ()

mark_as_advanced(
  PROTOBUF_INCLUDE_DIR
  PROTOBUF_LIBS
  PROTOBUF_STATIC_LIB
  PROTOBUF_PROTOC_EXECUTABLE
)

# This module defines
#  FLATBUF_INCLUDE_DIR, directory containing flatbuffer headers
#  FLATBUF_FOUND, whether flatbuffer has been found
#  FLATBUF_FLATC_EXECUTABLE, flatc file path

set(FLATBUF_SEARCH_HEADER_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/include/flatbuffers
)

set(FLATBUF_SEARCH_BIN_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/bin
)

find_path(FLATBUF_INCLUDE_DIR flatbuffers.h PATHS
  ${FLATBUF_SEARCH_HEADER_PATH}
  # make sure we don't accidentally pick up a different version
  NO_DEFAULT_PATH
)

find_program(FLATBUF_FLATC_EXECUTABLE NAMES flatc PATHS ${FLATBUF_SEARCH_BIN_PATH} NO_DEFAULT_PATH)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(flatbuffer DEFAULT_MSG FLATBUF_INCLUDE_DIR FLATBUF_FLATC_EXECUTABLE)

if (FLATBUF_INCLUDE_DIR AND FLATBUF_FLATC_EXECUTABLE)
  set(FLATBUF_FOUND TRUE)
else ()
  set(FLATBUF_FOUND FALSE)
endif ()

if (FLATBUF_FOUND)
  if (NOT FLATBUF_FIND_QUIETLY)
    message(STATUS "Found flatbuffer headers: ${FLATBUF_INCLUDE_DIR}")
    message(STATUS "Found flatc: ${FLATBUF_FLATC_EXECUTABLE}")
  endif ()
else ()
  message(STATUS "flatbuffer includes and libraries and flatc NOT found. "
    "Looked for headers in ${FLATBUF_SEARCH_HEADER_PATH}")
endif ()

mark_as_advanced(
  FLATBUF_INCLUDE_DIR
  FLATBUF_PROTOC_EXECUTABLE
)

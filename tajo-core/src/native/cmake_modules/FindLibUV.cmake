# Find libuv
#
# This module defines
# LIBUV_INCLUDE_DIR, where to find libuv include files, etc.
# LIBUV_LIBRARIES, the libraries to link against to use libuv.
# LIBUV_FOUND, If false, do not try to use libuv.

# also defined, but not for general use are
# LIBUV_LIBRARY, where to find the libuv library.

set(LIBUV_SEARCH_LIB_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/lib
)

find_path(LIBUV_INCLUDE_DIR uv.h
  PATHS ${CMAKE_SOURCE_DIR}/extlib/local/include
        NO_DEFAULT_PATH
  DOC   "Path to the ${LIBUV_H} file"
)


find_library(LIBUV_LIBRARY NAMES uv
  PATHS ${CMAKE_SOURCE_DIR}/extlib/local/lib
        NO_DEFAULT_PATH
  DOC   "libuv: Cross-platform asychronous I/O"
)

if(LIBUV_INCLUDE_DIR AND LIBUV_LIBRARY)
  set(LIBUV_LIBRARIES ${LIBUV_LIBRARY})
  set(LIBUV_STATIC_LIB ${LIBUV_SEARCH_LIB_PATH}/libuv.a)
  set(LIBUV_FOUND TRUE)
else(LIBUV_INCLUDE_DIR AND LIBUV_LIBRARY)
  set(LIBUV_FOUND FALSE)
endif(LIBUV_INCLUDE_DIR AND LIBUV_LIBRARY)

if(LIBUV_FOUND)
  if(NOT LIBUV_FIND_QUIETLY)
    message(STATUS "Found libuv: ${LIBUV_LIBRARIES}")
  endif(NOT LIBUV_FIND_QUIETLY)
else(LIBUV_FOUND)
  if(NOT LIBUV_FIND_QUIETLY)
    if(LIBUV_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find the libuv Library")
    else(LIBUV_FIND_REQUIRED)
      message(STATUS "Could not find the libuv Library")
    endif(LIBUV_FIND_REQUIRED)
  endif(NOT LIBUV_FIND_QUIETLY)
endif(LIBUV_FOUND)

mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARIES)


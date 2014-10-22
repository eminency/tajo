# This module defines
#  JSON_INCLUDE_DIR, directory containing headers
#  JSON_LIBS, directory containing gflag libraries
#  JSON_STATIC_LIB, path to libJSON.a
#  JSON_FOUND, whether JSON has been found

set(JSON_SEARCH_HEADER_PATHS  
  ${CMAKE_SOURCE_DIR}/extlib/local/include
)

set(JSON_SEARCH_LIB_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/lib
)

find_path(JSON_INCLUDE_DIR json_spirit.h PATHS
  ${JSON_SEARCH_HEADER_PATHS}
  # make sure we don't accidentally pick up a different version
  NO_DEFAULT_PATH
)

find_library(JSON_LIB_PATH NAMES json_spirit PATHS ${JSON_SEARCH_LIB_PATH})

if (JSON_LIB_PATH)
  set(JSON_FOUND TRUE)
  set(JSON_LIBS ${JSON_SEARCH_LIB_PATH})
  set(JSON_STATIC_LIB ${JSON_SEARCH_LIB_PATH}/libjson_spirit.a)
else ()
  set(JSON_FOUND FALSE)
endif ()

if (JSON_FOUND)
  if (NOT JSON_FIND_QUIETLY)
    message(STATUS "JSON found in ${JSON_SEARCH_LIB_PATH}")
    message(STATUS "JSON headers found in ${JSON_SEARCH_HEADER_PATHS}")
  endif ()
else ()
  message(STATUS "JSON includes and libraries NOT found. "
    "Looked for headers in ${JSON_SEARCH_HEADER_PATHS}, "
    "and for libs in ${JSON_SEARCH_LIB_PATH}")
endif ()

mark_as_advanced(
  JSON_INCLUDE_DIR
  JSON_LIBS
  JSON_STATIC_LIB
)

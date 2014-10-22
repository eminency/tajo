# This module defines
#  RE2_INCLUDE_DIR, directory containing headers
#  RE2_LIBS, directory containing gflag libraries
#  RE2_STATIC_LIB, path to libRE2.a
#  RE2_FOUND, whether RE2 has been found

set(RE2_SEARCH_HEADER_PATHS  
  ${CMAKE_SOURCE_DIR}/extlib/local/include
)

set(RE2_SEARCH_LIB_PATH
  ${CMAKE_SOURCE_DIR}/extlib/local/lib
)

find_path(RE2_INCLUDE_DIR re2/re2.h PATHS
  ${RE2_SEARCH_HEADER_PATHS}
  # make sure we don't accidentally pick up a different version
  NO_DEFAULT_PATH
)

find_library(RE2_LIB_PATH NAMES re2 PATHS ${RE2_SEARCH_LIB_PATH})

if (RE2_LIB_PATH)
  set(RE2_FOUND TRUE)
  set(RE2_LIBS ${RE2_SEARCH_LIB_PATH})
  set(RE2_STATIC_LIB ${RE2_SEARCH_LIB_PATH}/libre2.a)
else ()
  set(RE2_FOUND FALSE)
endif ()

if (RE2_FOUND)
  if (NOT RE2_FIND_QUIETLY)
    message(STATUS "RE2 found in ${RE2_SEARCH_LIB_PATH}")
    message(STATUS "RE2 headers found in ${RE2_SEARCH_HEADER_PATHS}")
  endif ()
else ()
  message(STATUS "RE2 includes and libraries NOT found. "
    "Looked for headers in ${RE2_SEARCH_HEADER_PATHS}, "
    "and for libs in ${RE2_SEARCH_LIB_PATH}")
endif ()

mark_as_advanced(
  RE2_INCLUDE_DIR
  RE2_LIBS
  RE2_STATIC_LIB
)

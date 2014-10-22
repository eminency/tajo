# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

# add this options before PROJECT keyword
# set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(tajo_jitvec)

# Enabling CTEST
enable_testing()

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF ()

# Set compile flags based on the build type.
STRING (TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
  SET(CMAKE_CXX_FLAGS ${CXX_FLAGS_DEBUG})
  SET(BINARY_OUTPUT_DIR "${PROJECT_BINARY_DIR}/Debug/")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
  SET(CMAKE_CXX_FLAGS ${CXX_FLAGS_RELEASE})
  SET(BINARY_OUTPUT_DIR "${PROJECT_BINARY_DIR}/Release/")
endif ()


add_definitions(-std=c++11)
#if(CMAKE_COMPILER_IS_GNUCXX)
#  add_definitions(-std=c++11)
#endif()

#SET(CMAKE_CXX_FLAGS "${CXX_COMMON_FLAGS} ${CMAKE_CXX_FLAGS}")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#message(STATUS ${CMAKE_CXX_FLAGS})

# Detect Mac OS X. If then, it specify CXX FLAGS for libc++.
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(OperatingSystem "Mac OS X")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

SET(LIBRARY_OUTPUT_PATH "${BINARY_OUTPUT_DIR}/lib")
SET(EXECUTABLE_OUTPUT_PATH "${BINARY_OUTPUT_DIR}/bin")

# where to find cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

# Detect CPU features like SSE3, SSE4.2, AVX, and AVX2
include (${CMAKE_SOURCE_DIR}/cmake_modules/FindSSE.cmake)
FindSSE()

if (SSE4_2_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4")
endif()

# Find glog
find_package(GLog REQUIRED)
include_directories(${GLOG_INCLUDE_DIR})
set(LIBS ${LIBS} ${GLOG_LIBS})
add_library(glogstatic STATIC IMPORTED)
set_target_properties(glogstatic PROPERTIES IMPORTED_LOCATION ${GLOG_STATIC_LIB})

# Find gflags
find_package(GFlags REQUIRED)
include_directories(${GFLAGS_INCLUDE_DIR})
set(LIBS ${LIBS} ${GFLAGS_LIBS})
add_library(gflagsstatic STATIC IMPORTED)
set_target_properties(gflagsstatic PROPERTIES IMPORTED_LOCATION ${GFLAGS_STATIC_LIB})

# Find Flatbuffer
#find_package(Flatbuffer REQUIRED)
#include_directories(${FLATBUF_INCLUDE_DIR})

# Find ProtoBuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
set(LIBS ${LIBS} ${PROTOBUF_LIBS})
add_library(protobufstatic STATIC IMPORTED)
set_target_properties(protobufstatic PROPERTIES IMPORTED_LOCATION ${PROTOBUF_STATIC_LIB})

# find boost headers and libs
set(Boost_DEBUG TRUE)
# in order to load libbackend.so into the jvm, we need to link against a shared
# libboost_thread-mt.so
#set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system thread date_time filesystem)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
message(STATUS ${Boost_INCLUDE_DIRS})
message(STATUS ${Boost_LIBRARIES})

# Find LLVM
find_package(Llvm REQUIRED)
include_directories(BEFORE ${LLVM_INCLUDE_DIR})
set(LIBS ${LIBS} ${LLVM_MODULE_LIBS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CFLAGS}")


# Find JsonSprits
find_package(Json REQUIRED)
include_directories(${JSON_INCLUDE_DIR})
set(LIBS ${LIBS} ${JSON_LIBS})
add_library(jsonstatic STATIC IMPORTED)
set_target_properties(jsonstatic PROPERTIES IMPORTED_LOCATION ${JSON_STATIC_LIB})

# Find HDFS
find_package(HDFS REQUIRED)
include_directories(${HDFS_INCLUDE_DIR})
set(LIBS ${LIBS} ${HDFS_LIBS})
add_library(hdfsstatic STATIC IMPORTED)
set_target_properties(hdfsstatic PROPERTIES IMPORTED_LOCATION ${HDFS_STATIC_LIB})

add_subdirectory(src)

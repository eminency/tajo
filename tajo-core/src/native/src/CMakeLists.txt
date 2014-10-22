set(GENERATED_SRC_DIR "${PROJECT_BINARY_DIR}/generated")
SET(PROTOBUF_GEN_DIR "${GENERATED_SRC_DIR}/proto")
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Preparing the dir for generated sources
file(MAKE_DIRECTORY ${GENERATED_SRC_DIR}) 

# Run protoc to generate *.pb.cc files.
set(PROTO_OUTPUT)
FUNCTION(ADD_PROTO PROTO_NAME)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${PROTOBUF_GEN_DIR}/${PROTO_NAME}.pb.cc
    COMMAND mkdir -p "${PROTOBUF_GEN_DIR}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${SRC_ROOT}/proto/${PROTO_NAME}.proto -I${SRC_ROOT}/proto --cpp_out="${PROTOBUF_GEN_DIR}"
    DEPENDS ${SRC_ROOT}/proto/${PROTO_NAME}.proto
  )
  ADD_CUSTOM_TARGET(${PROTO_NAME}_pb_compile ALL DEPENDS ${PROTOBUF_GEN_DIR}/${PROTO_NAME}.pb.cc)
  SET(PROTO_OUTPUT ${PROTO_OUTPUT} ${PROTOBUF_GEN_DIR}/${PROTO_NAME}.pb.cc PARENT_SCOPE)
ENDFUNCTION()

ADD_PROTO(DataTypes)
ADD_PROTO(CatalogProtos)
ADD_PROTO(RpcProtos)
ADD_PROTO(TestProtos)
ADD_PROTO(TestProtocol)
ADD_PROTO(JitVecTestProtocol)

include_directories(BEFORE 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${PROTOBUF_GEN_DIR}
)

add_library(jitvec
  common/status.cc
  common/schema.cc
  common/datatypes.cc
  common/stringpiece.cc

  eval/eval_node.cc
  
  plan/logical_node.cc
  plan/logical_plan_builder.cc

  exec/delim_fields_parser.cc
  exec/text_scanner.cc

  io/in_mem_stream.cc
  io/file_stream.cc
  io/hdfs_file_stream.cc
  
  rpc/tcp_rpc_channel.cc

  util/memory.cc

  vector/vectors.cc
  vector/map_plus_primitives.cc

  ${PROTO_OUTPUT}
)

target_link_libraries(jitvec
  ${GLOG_LIBS}
  ${Boost_LIBRARIES}
  ${PROTOBUF_LIBS}
  jsonstatic
)

add_executable(tajod
  main/engine.cc
)

target_link_libraries(tajod
  ${GLOG_LIBS}
  ${Boost_LIBRARIES}

  jitvec
)

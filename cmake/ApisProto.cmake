include(FetchContent)

FetchContent_Declare(
  apis_proto
  GIT_REPOSITORY https://github.com/secretflow/secure-data-capsule-apis.git
  GIT_TAG bf3a19c4eddb0e2cf4c9c21c134413dffdf321c9)

FetchContent_GetProperties(apis_proto)
if(NOT apis_proto_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(apis_proto)

  set(IMPORT_DIRS ${apis_proto_SOURCE_DIR})
  set(PROTOC_OUT_DIR ${apis_proto_BINARY_DIR}/generated)
  file(GLOB UAL_PROTOS ${apis_proto_SOURCE_DIR}/secretflowapis/v2/sdc/ual.proto)
  set(PROTOBUF_PROTOC_EXECUTABLE protoc)
  make_directory(${PROTOC_OUT_DIR})

  foreach(proto ${UAL_PROTOS})
    message("STATUS ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${IMPORT_DIRS} --cpp_out=${PROTOC_OUT_DIR} ${proto}")
    execute_process(
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${IMPORT_DIRS}
              --cpp_out=${PROTOC_OUT_DIR} ${proto} RESULT_VARIABLE rv)
    # Optional, but that can show the user if something have gone wrong with the
    # proto generation
    if(${rv})
      message("Generation of data model returned ${rv} for proto ${proto}")
    endif()
  endforeach()

  file(GLOB_RECURSE PROTO_SOURCES "${PROTOC_OUT_DIR}/**/*.cc")
  add_library(ual_proto STATIC ${PROTO_SOURCES})
  add_dependencies(ual_proto protobuf)
  target_include_directories(ual_proto PUBLIC ${PROTOC_OUT_DIR})
  target_link_libraries(ual_proto PUBLIC protobuf::libprotobuf)
  target_compile_options(ual_proto PUBLIC -fPIC)
endif()

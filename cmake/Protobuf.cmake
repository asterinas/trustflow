include(ExternalProject)

set(LIBPROTOBUF_ROOT ${CMAKE_BINARY_DIR}/external/protobuf)
set(LIBPROTOBUF_SRC_PATH ${LIBPROTOBUF_ROOT}/src/protobuf)
set(LIBPROTOBUF_BINARY_PATH ${LIBPROTOBUF_ROOT}/build)
set(LIBPROTOBUF_INC_PATH ${LIBPROTOBUF_BINARY_PATH}/include)


if(WASM)
  message("Using emscripten!")

  ExternalProject_Add(
    protobuf
    URL https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.19.4.tar.gz
    PREFIX ${LIBPROTOBUF_ROOT}
    CONFIGURE_COMMAND
      cd ${LIBPROTOBUF_SRC_PATH}/cmake && emcmake cmake
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -Dprotobuf_BUILD_PROTOC_BINARIES:BOOL=OFF -Dprotobuf_BUILD_TESTS:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX=${LIBPROTOBUF_BINARY_PATH}
    BUILD_COMMAND cd ${LIBPROTOBUF_SRC_PATH}/cmake && emmake make
    INSTALL_COMMAND cd ${LIBPROTOBUF_SRC_PATH}/cmake && make install)
  add_library(protobuf::libprotobuf STATIC IMPORTED)
  set_property(
    TARGET protobuf::libprotobuf
    PROPERTY IMPORTED_LOCATION ${LIBPROTOBUF_BINARY_PATH}/lib/libprotobuf.a)
  make_directory(${LIBPROTOBUF_INC_PATH})
  target_include_directories(protobuf::libprotobuf
                             INTERFACE ${LIBPROTOBUF_INC_PATH})
else()
ExternalProject_Add(
  protobuf
  URL https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.19.4.tar.gz
  PREFIX ${LIBPROTOBUF_ROOT}
  CONFIGURE_COMMAND
    cd ${LIBPROTOBUF_SRC_PATH}/cmake &&  cmake
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -Dprotobuf_BUILD_PROTOC_BINARIES:BOOL=OFF -Dprotobuf_BUILD_TESTS:BOOL=OFF
    -DCMAKE_INSTALL_PREFIX=${LIBPROTOBUF_BINARY_PATH}
  BUILD_COMMAND cd ${LIBPROTOBUF_SRC_PATH}/cmake &&  make
  INSTALL_COMMAND cd ${LIBPROTOBUF_SRC_PATH}/cmake && make install)
add_library(protobuf::libprotobuf STATIC IMPORTED)
set_property(
  TARGET protobuf::libprotobuf
  PROPERTY IMPORTED_LOCATION ${LIBPROTOBUF_BINARY_PATH}/lib/libprotobuf.a)
make_directory(${LIBPROTOBUF_INC_PATH})
target_include_directories(protobuf::libprotobuf
                           INTERFACE ${LIBPROTOBUF_INC_PATH})
endif()

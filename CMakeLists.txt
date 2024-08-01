cmake_minimum_required(VERSION 3.5.1)

# Define build mode
set(BUILD_MODE
    "wasm"
    CACHE STRING "Select build mode for libverification(host|wasm)")

# Print build mode
message(STATUS "Build Mode: ${BUILD_MODE}")


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(BUILD_MODE STREQUAL "wasm")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Emscripten.cmake")
  set(WASM 1)
  add_definitions(-DWASM)
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -sMAIN_MODULE=2 -fPIC -fexceptions -O3")
elseif()

endif()

project(trustflow)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Abseil)
include(ApisProto)
include(Cppcodec)
include(DcapVerifier)
include(Fmtlib)
include(Openssl)
include(Protobuf)
include(SgxHeaders)
include(Yacl)
include(Spdlog)
include(CsvHeader)
# include(Pybind)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(trustflow/attestation/verification)
add_subdirectory(pylib/trustflow/attestation/verification)

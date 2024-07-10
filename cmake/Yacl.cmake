include(FetchContent)

FetchContent_Declare(
  yacl
  URL https://github.com/secretflow/yacl/archive/refs/tags/0.4.5b0.tar.gz)

FetchContent_GetProperties(yacl)
if(NOT yacl_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(yacl)

  add_library(yacl_exception INTERFACE)
  target_link_libraries(yacl_exception INTERFACE absl::debugging
                                                 absl::type_traits fmt::fmt)
  target_include_directories(yacl_exception INTERFACE ${yacl_SOURCE_DIR})

  add_library(yacl_buffer STATIC ${yacl_SOURCE_DIR}/yacl/base/buffer.cc)
  target_link_libraries(yacl_buffer PUBLIC yacl_exception)

  add_library(yacl_crypto STATIC
              ${yacl_SOURCE_DIR}/yacl/crypto/hash/ssl_hash.cc)
  target_link_libraries(yacl_crypto PUBLIC yacl_buffer OpenSSL::Crypto)
endif()

include(FetchContent)

FetchContent_Declare(
  yacl
  GIT_REPOSITORY https://github.com/secretflow/yacl.git
  GIT_TAG c45fb48dff630a286953bc21bfae5e7d083ae422)

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
              ${yacl_SOURCE_DIR}/yacl/crypto/base/hash/ssl_hash.cc)
  add_dependencies(yacl_crypto openssl)
  target_link_libraries(yacl_crypto PUBLIC yacl_buffer libopenssl)
endif()

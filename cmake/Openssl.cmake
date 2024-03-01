include(ExternalProject)

if(WASM)
  message("Using emscripten!")

  set(LIBOPENSSL_ROOT ${CMAKE_BINARY_DIR}/external/openssl)
  set(LIBOPENSSL_SRC_PATH ${LIBOPENSSL_ROOT}/src/openssl)
  set(LIBOPENSSL_BINARY_PATH ${LIBOPENSSL_ROOT}/build)
  set(LIBOPENSSL_INC_PATH ${LIBOPENSSL_BINARY_PATH}/include)
  set(LIBOPENSSL_LIB_PATH ${LIBOPENSSL_BINARY_PATH}/lib)

  ExternalProject_Add(
    openssl
    URL https://www.openssl.org/source/openssl-3.0.12.tar.gz
    PREFIX ${LIBOPENSSL_ROOT}
    CONFIGURE_COMMAND
      cd ${LIBOPENSSL_SRC_PATH} && emconfigure ./Configure
      --prefix=${LIBOPENSSL_BINARY_PATH} linux-x86 no-asm no-threads no-engine
      no-hw no-weak-ssl-ciphers no-dtls no-shared no-dso no-tests no-unit-test
      && sed -i -e "s|^CROSS_COMPILE.*$|CROSS_COMPILE=|g" Makefile
    BUILD_COMMAND cd ${LIBOPENSSL_SRC_PATH} && emmake make -j4 build_generated
                  libssl.a libcrypto.a
    INSTALL_COMMAND cd ${LIBOPENSSL_SRC_PATH} && make install)

  add_library(libopenssl STATIC IMPORTED)
  set_property(TARGET libopenssl PROPERTY IMPORTED_LOCATION
                                          ${LIBOPENSSL_LIB_PATH}/libcrypto.a)
  make_directory(${LIBOPENSSL_INC_PATH})
  target_include_directories(libopenssl INTERFACE ${LIBOPENSSL_INC_PATH})
else()
  message("Not using emscripten!")
endif()

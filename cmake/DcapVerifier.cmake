include(ExternalProject)

set(LIBDCAP_ROOT ${CMAKE_BINARY_DIR}/external/dcap)
set(LIBDCAP_SRC_PATH ${LIBDCAP_ROOT}/src/dcap)
set(LIBDCAP_BINARY_PATH ${LIBDCAP_ROOT}/build)
set(LIBDCAP_INC_PATH ${LIBDCAP_BINARY_PATH}/include)

if(WASM)
  message("Dcap: Using emscripten!")

  ExternalProject_Add(
    dcap
    URL https://github.com/intel/SGXDataCenterAttestationPrimitives/archive/refs/tags/DCAP_1.19.tar.gz
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patch/dcap.patch
    PREFIX ${LIBDCAP_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
      cd ${LIBDCAP_SRC_PATH} && emmake make GEN_STATIC=1 GEN_DYNAMIC=0 WASM=1 -C
      QuoteGeneration/qcnl/linux && emmake make GEN_STATIC=1 GEN_DYNAMIC=0
      WASM=1 -C QuoteGeneration/qpl/linux && emmake make GEN_STATIC=1
      GEN_DYNAMIC=0 WASM=1 -C QuoteVerification/dcap_quoteverify/linux
    INSTALL_COMMAND "")

  add_library(dcap::qcnl STATIC IMPORTED)
  set_property(
    TARGET dcap::qcnl
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteGeneration/qcnl/linux/libsgx_default_qcnl_wrapper.a
  )

  add_library(dcap::qpl STATIC IMPORTED)
  set_property(
    TARGET dcap::qpl
    PROPERTY IMPORTED_LOCATION
             ${LIBDCAP_SRC_PATH}/QuoteGeneration/qpl/linux/libdcap_quoteprov.a)

  add_library(dcap::qvl_attestation STATIC IMPORTED)
  set_property(
    TARGET dcap::qvl_attestation
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_qvl_attestation.a
  )

  add_library(dcap::qvl_parser STATIC IMPORTED)
  set_property(
    TARGET dcap::qvl_parser
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_qvl_parser.a
  )

  add_library(dcap::quoteverify STATIC IMPORTED)
  set_property(
    TARGET dcap::quoteverify
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_quoteverify.a
  )
  set_property(
    TARGET dcap::quoteverify
    PROPERTY INTERFACE_LINK_LIBRARIES dcap::qpl dcap::qvl_attestation
             dcap::qvl_parser dcap::qcnl)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${LIBDCAP_SRC_PATH}/QuoteVerification/QvE/Include/* ${LIBDCAP_INC_PATH}/)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${LIBDCAP_SRC_PATH}/QuoteGeneration/qpl/inc/* ${LIBDCAP_INC_PATH}/)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/inc/*
      ${LIBDCAP_INC_PATH}/)

  make_directory(${LIBDCAP_INC_PATH})
  target_include_directories(dcap::quoteverify INTERFACE ${LIBDCAP_INC_PATH})

else()

  ExternalProject_Add(
    dcap
    URL https://github.com/intel/SGXDataCenterAttestationPrimitives/archive/refs/tags/DCAP_1.19.tar.gz
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patch/dcap2.patch
    PREFIX ${LIBDCAP_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
      cd ${LIBDCAP_SRC_PATH} && make GEN_STATIC=1 GEN_DYNAMIC=0 -C
      QuoteGeneration/qcnl/linux &&  make GEN_STATIC=1 GEN_DYNAMIC=0
       -C QuoteGeneration/qpl/linux &&  make GEN_STATIC=1
      GEN_DYNAMIC=0  -C QuoteVerification/dcap_quoteverify/linux
    INSTALL_COMMAND "")

  add_library(dcap::qcnl STATIC IMPORTED)
  set_property(
    TARGET dcap::qcnl
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteGeneration/qcnl/linux/libsgx_default_qcnl_wrapper.a
  )

  add_library(dcap::qpl STATIC IMPORTED)
  set_property(
    TARGET dcap::qpl
    PROPERTY IMPORTED_LOCATION
            ${LIBDCAP_SRC_PATH}/QuoteGeneration/qpl/linux/libdcap_quoteprov.a)

  add_library(dcap::qvl_attestation STATIC IMPORTED)
  set_property(
    TARGET dcap::qvl_attestation
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_qvl_attestation.a
  )

  add_library(dcap::qvl_parser STATIC IMPORTED)
  set_property(
    TARGET dcap::qvl_parser
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_qvl_parser.a
  )

  add_library(dcap::quoteverify STATIC IMPORTED)
  set_property(
    TARGET dcap::quoteverify
    PROPERTY
      IMPORTED_LOCATION
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/linux/libsgx_dcap_quoteverify.a
  )
  set_property(
    TARGET dcap::quoteverify
    PROPERTY INTERFACE_LINK_LIBRARIES dcap::qpl dcap::qvl_attestation
            dcap::qvl_parser dcap::qcnl)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${LIBDCAP_SRC_PATH}/QuoteVerification/QvE/Include/* ${LIBDCAP_INC_PATH}/)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${LIBDCAP_SRC_PATH}/QuoteGeneration/qpl/inc/* ${LIBDCAP_INC_PATH}/)

  add_custom_command(
    TARGET dcap
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${LIBDCAP_SRC_PATH}/QuoteVerification/dcap_quoteverify/inc/*
      ${LIBDCAP_INC_PATH}/)

  make_directory(${LIBDCAP_INC_PATH})
  target_include_directories(dcap::quoteverify INTERFACE ${LIBDCAP_INC_PATH})
endif()

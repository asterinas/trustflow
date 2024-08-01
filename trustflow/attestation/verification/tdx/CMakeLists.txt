set(SOURCES tdx_verifier.cc)

add_library(tdx_verifier STATIC ${SOURCES})
target_link_libraries(
  tdx_verifier
  ual_proto
  yacl_buffer
  cppcodec
  sgx_headers
  dcap::quoteverify
  yacl_crypto)
add_dependencies(tdx_verifier dcap)

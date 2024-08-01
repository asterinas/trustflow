set(SOURCES sgx2_verifier.cc)

add_library(sgx2_verifier STATIC ${SOURCES})
target_link_libraries(
  sgx2_verifier
  ual_proto
  yacl_buffer
  cppcodec
  sgx_headers
  dcap::quoteverify
  yacl_crypto)
add_dependencies(sgx2_verifier dcap)

set(SOURCES verifier_wrapper.cc)

add_library(verifier_wrapper STATIC ${SOURCES})
target_link_libraries(
  verifier_wrapper
  verifier_interface
  sgx2_verifier
  tdx_verifier
  csv_verifier
)
add_dependencies(verifier_wrapper dcap)

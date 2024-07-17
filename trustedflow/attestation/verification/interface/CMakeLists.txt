set(SOURCES verifier.cc)

add_library(verifier_interface STATIC ${SOURCES})
target_link_libraries(
  verifier_interface
  ual_proto
  yacl_crypto)

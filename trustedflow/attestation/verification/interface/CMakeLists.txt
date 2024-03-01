set(SOURCES verifier.cc)

add_library(verifier STATIC ${SOURCES})
target_link_libraries(
  verifier
  ual_proto
  yacl_crypto)

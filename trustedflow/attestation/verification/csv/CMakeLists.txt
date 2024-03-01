set(SOURCES csv_verifier.cc)

add_library(csv_verifier STATIC ${SOURCES})
target_link_libraries(
  csv_verifier
  ual_proto
  yacl_buffer
  cppcodec
  yacl_crypto spdlog::spdlog_header_only)

target_include_directories(csv_verifier PUBLIC ${CSV_HEADER_INCLUDE})

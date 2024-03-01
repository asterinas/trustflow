set(SOURCES wasm_binding.cc)

add_subdirectory(interface)
add_subdirectory(sgx2)
add_subdirectory(tdx)
add_subdirectory(csv)
add_subdirectory(wrapper)

add_executable(wasm_binding ${SOURCES})
target_link_libraries(wasm_binding PUBLIC verifier sgx2_verifier tdx_verifier csv_verifier)

set(WASM_MAIN_BASE_FLAGS "-sMAIN_MODULE=2 -fPIC -fexceptions")
set(WASM_COMMON_FLAGS
    "-sALLOW_MEMORY_GROWTH=1 -sSAFE_HEAP=1 -fexceptions -fPIC -Wno-limited-postlink-optimizations -Wno-linkflags -O3"
)
set(WASM_MAIN_NORMAL_FLAGS
    "${WASM_COMMON_FLAGS} -sMAIN_MODULE=2 --use-preload-plugins -lembind -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$AsciiToString"
)
set(WASM_MAIN_ASYNC_FLAGS
    "${WASM_MAIN_NORMAL_FLAGS} -gsource-map -sASYNCIFY -sASYNCIFY_IMPORTS=['dcap_fetch_proxy'] -sSINGLE_FILE"
)

target_link_libraries(wasm_binding PUBLIC libopenssl absl::type_traits
                                          absl::symbolize spdlog::spdlog)
set_target_properties(
  wasm_binding
  PROPERTIES COMPILE_FLAGS ${WASM_MAIN_BASE_FLAGS}
             LINK_FLAGS "${WASM_MAIN_ASYNC_FLAGS}"
             SUFFIX ".js")

add_subdirectory(interface)
add_subdirectory(sgx2)
add_subdirectory(tdx)
add_subdirectory(csv)
add_subdirectory(wrapper)

if(WASM)

set(SOURCES wasm_binding.cc)

add_executable(trustflow_verifier ${SOURCES})
target_link_libraries(trustflow_verifier PUBLIC verifier_wrapper)

set(WASM_MAIN_BASE_FLAGS "-sMAIN_MODULE=2 -fPIC -fexceptions")
set(WASM_COMMON_FLAGS
    "-sALLOW_MEMORY_GROWTH=1 -sSAFE_HEAP=1 -fexceptions -fPIC -Wno-limited-postlink-optimizations -Wno-linkflags -O3"
)
set(WASM_MAIN_NORMAL_FLAGS
    "${WASM_COMMON_FLAGS} -sMAIN_MODULE=2 --use-preload-plugins -lembind -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$AsciiToString"
)
set(WASM_MAIN_ASYNC_FLAGS
    "${WASM_MAIN_NORMAL_FLAGS} -gsource-map -sASYNCIFY -sASYNCIFY_IMPORTS=['dcap_fetch_proxy'] -sSINGLE_FILE -s EXPORT_ES6=1 -s MODULARIZE=1"
)

target_link_libraries(trustflow_verifier PUBLIC OpenSSL::Crypto absl::type_traits
                                          absl::symbolize spdlog::spdlog)
set_target_properties(
    trustflow_verifier
  PROPERTIES COMPILE_FLAGS ${WASM_MAIN_BASE_FLAGS}
             LINK_FLAGS "${WASM_MAIN_ASYNC_FLAGS}"
             SUFFIX ".js")

endif()

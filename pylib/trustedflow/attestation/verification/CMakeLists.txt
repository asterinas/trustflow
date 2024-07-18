
find_package(pybind11 REQUIRED)

pybind11_add_module(verifier verifier_modules.cc)
target_link_libraries(verifier PUBLIC verifier_wrapper)
target_link_libraries(verifier PUBLIC OpenSSL::Crypto absl::type_traits
                                          absl::symbolize spdlog::spdlog)
include(FetchContent)

FetchContent_Declare(
  fmtlib URL https://github.com/fmtlib/fmt/archive/refs/tags/10.1.1.tar.gz)

set(fmtlib_FMT_TEST
    OFF
    CACHE INTERNAL "")

FetchContent_MakeAvailable(fmtlib)

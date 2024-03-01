include(FetchContent)

FetchContent_Declare(
  cppcodec
  GIT_REPOSITORY https://github.com/tplgy/cppcodec.git
  GIT_TAG 9f67d7026d3dee8fc6a0af614d97f9365cee2872)

FetchContent_GetProperties(cppcodec)
if(NOT cppcodec_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(cppcodec)

  add_library(cppcodec INTERFACE)
  target_include_directories(cppcodec INTERFACE ${cppcodec_SOURCE_DIR})
endif()

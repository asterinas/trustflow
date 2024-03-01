set(CSV_HEADER_FILE_PATH
    ${CMAKE_BINARY_DIR}/external/hygon-devkit/csv/attestation/attestation.h)
set(CSV_HEADER_INCLUDE ${CMAKE_BINARY_DIR}/external/hygon-devkit)

find_program(SED_EXECUTEABLE NAMES sed)

file(
  DOWNLOAD
  https://gitee.com/anolis/hygon-devkit/raw/master/csv/attestation/attestation.h
  ${CSV_HEADER_FILE_PATH})

execute_process(COMMAND ${SED_EXECUTEABLE} -i "/ioctl.h/ d"
                        ${CSV_HEADER_FILE_PATH})

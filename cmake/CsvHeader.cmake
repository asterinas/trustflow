set(CSV_HEADER_FILE_PATH
    ${CMAKE_BINARY_DIR}/external/hygon-devkit/csv/attestation/attestation.h)
set(CSV_HEADER_INCLUDE ${CMAKE_BINARY_DIR}/external/hygon-devkit)

find_program(SED_EXECUTEABLE NAMES sed)

file(
  DOWNLOAD
  https://gitee.com/anolis/hygon-devkit/raw/2683fd4577a1d2e0fc19247b47a6ca68f4879c33/csv/attestation/attestation.h
  ${CSV_HEADER_FILE_PATH})

execute_process(COMMAND ${SED_EXECUTEABLE} -i "/ioctl.h/ d"
                        ${CSV_HEADER_FILE_PATH})

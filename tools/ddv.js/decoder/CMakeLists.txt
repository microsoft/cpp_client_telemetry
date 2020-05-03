cmake_minimum_required (VERSION 2.6)

project (decoder)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11")

set(WORKSPACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

add_executable(decoder src/decoder.cpp ${WORKSPACE_ROOT}/lib/decoder/PayloadDecoder.cpp)

include_directories(
    ${WORKSPACE_ROOT}/lib/include/public
    ${WORKSPACE_ROOT}/lib/include/mat
    ${WORKSPACE_ROOT}/lib/include
    ${WORKSPACE_ROOT}/lib
)

if (EMSCRIPTEN)
  # Emscripten target
  set(SYSROOT $ENV{SYSROOT})
  include_directories(${SYSROOT}/zlib ${SYSROOT}/zlib/build)
  set(LIBZ ${SYSROOT}/zlib/build/libz.a)
  message("-- BUILD_EMSCRIPTEN")
  message("-- SYSROOT=${SYSROOT}")
  message("-- ZLIB=${LIBZ}")
  target_link_libraries(decoder ${LIBZ})
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DEMSCRIPTEN")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMSCRIPTEN")
else()
  message("-- BUILD_HOST ${CMAKE_SYSTEM_PROCESSOR}")
  # Regular target
  target_link_libraries(decoder z)
endif()

cmake_minimum_required (VERSION 2.6)

project (decoder)
set(SYSROOT $ENV{SYSROOT})
include_directories(${SYSROOT}/zlib ${SYSROOT}/zlib/build)

add_executable(decoder src/decoder.cpp)

if (EMSCRIPTEN)
  # Emscripten target
  set(LIBZ ${SYSROOT}/zlib/build/libz.a)
  message("-- BUILD_EMSCRIPTEN")
  message("-- SYSROOT=${SYSROOT}")
  message("-- ZLIB=${LIBZ}")
  target_link_libraries(decoder ${LIBZ})
else()
  message("-- BUILD_HOST ${CMAKE_SYSTEM_PROCESSOR}")
  # Regular target
  target_link_libraries(decoder z)
endif()

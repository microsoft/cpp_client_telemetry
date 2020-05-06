cmake_minimum_required (VERSION 2.6)

project (decoder)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11")

set(WORKSPACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

add_executable(decoder src/decoder.cpp ${WORKSPACE_ROOT}/lib/decoder/PayloadDecoder.cpp)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${WORKSPACE_ROOT}/lib/include/public
    ${WORKSPACE_ROOT}/lib/include/mat
    ${WORKSPACE_ROOT}/lib/include
    ${WORKSPACE_ROOT}/lib
)

if (EMSCRIPTEN)
  # Emscripten target
  set(SYSROOT $ENV{SYSROOT})
  include_directories("${SYSROOT}/zlib" "${SYSROOT}/zlib/build")
  set(LIBZ "${SYSROOT}/zlib/build/libz.a")
  message("-- BUILD_EMSCRIPTEN")
  message("-- SYSROOT=${SYSROOT}")
  message("-- ZLIB=${LIBZ}")
  target_link_libraries(decoder ${LIBZ})
  # TODO: -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=2 - ref: https://developers.google.com/web/updates/2018/10/wasm-threads
  # NOT WORKING: -s WASM_BACKEND -s ASYNCIFY
  set(EMCC_FLAGS "${EMCC_FLAGS} -s WASM=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s EXPORTED_FUNCTIONS=\"['_main']\" -s EXTRA_EXPORTED_RUNTIME_METHODS=\"['addFunction','getMemory','lengthBytesUTF8','stringToUTF8','UTF8ToString']\" ")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DEMSCRIPTEN ${EMCC_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMSCRIPTEN ${EMCC_FLAGS}")
else()
  message("-- BUILD_HOST ${CMAKE_SYSTEM_PROCESSOR}")
  # Regular target
  target_link_libraries(decoder z)
endif()

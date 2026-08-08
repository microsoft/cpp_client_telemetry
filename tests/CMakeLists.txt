include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite )

set(MATSDK_GTEST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googletest/include)
set(MATSDK_GMOCK_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googlemock/include)
if(NOT EXISTS "${MATSDK_GTEST_INCLUDE_DIR}/gtest/gtest.h")
  message(FATAL_ERROR
    "Tests require the third_party/googletest submodule at "
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest.")
endif()

add_library(matsdk_test_includes INTERFACE)
target_include_directories(matsdk_test_includes INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder
  ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite
  ${MATSDK_GTEST_INCLUDE_DIR}
  ${MATSDK_GMOCK_INCLUDE_DIR})

set(TESTS_COMMON_SRCS
  ../common/Common.cpp
  ../common/Mocks.cpp
  ../common/Reactor.cpp
  ../../lib/decoder/PayloadDecoder.cpp
)

if(BUILD_FUNC_TESTS)
  add_subdirectory(functests)
endif()

if(BUILD_UNIT_TESTS)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/unittests)
  add_subdirectory(unittests)
endif()

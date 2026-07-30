add_library(matsdk_test_config INTERFACE)
target_link_libraries(matsdk_test_config INTERFACE matsdk_internal_config)
target_include_directories(matsdk_test_config INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pal
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googletest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googlemock/include)
if(NOT MATSDK_USES_NLOHMANN_TARGET)
  target_include_directories(matsdk_test_config INTERFACE ${PROJECT_SOURCE_DIR})
endif()

if(NOT TARGET gtest OR NOT TARGET gmock)
  message(FATAL_ERROR "gtest/gmock targets were not configured.")
endif()

set(TESTS_COMMON_SRCS
  ../common/Common.cpp
  ../common/Mocks.cpp
  ../common/Reactor.cpp
  ../../lib/decoder/PayloadDecoder.cpp
)

if(MATSDK_BUILD_FUNC_TESTS)
  add_subdirectory(functests)
endif()

if(MATSDK_BUILD_UNIT_TESTS)
  target_include_directories(matsdk_test_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/unittests)
  add_subdirectory(unittests)
endif()

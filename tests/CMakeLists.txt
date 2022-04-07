include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googletest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googlemock/include
)

include_directories(../lib)

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

include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite )

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/build)
  # Prefer new Google Test submodule if built in source tree (required for Ubuntu 20.04 and above)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googletest/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googlemock/include
  )
else()
  # Use old Google Test location. This location will be eventually removed.
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../googletest/googletest/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../googletest/googlemock/include
  )
endif()

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

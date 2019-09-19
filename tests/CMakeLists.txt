include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/../googletest/googletest/include ${CMAKE_CURRENT_SOURCE_DIR}/../googletest/googlemock/include ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite )

include_directories(../lib)

set(TESTS_COMMON_SRCS
  ../common/Common.cpp
  ../common/Mocks.cpp
  ../../lib/decoder/PayloadDecoder.cpp
)

if(BUILD_FUNC_TESTS)
  add_subdirectory(functests)
endif()

if(BUILD_UNIT_TESTS)
  add_subdirectory(unittests)
endif()

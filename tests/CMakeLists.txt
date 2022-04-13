include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/public ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/../lib/decoder ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite )
message("-->Lalit tests")

include_directories(../lib)

set(TESTS_COMMON_SRCS
  ../common/Common.cpp
  ../common/Mocks.cpp
  ../common/Reactor.cpp
  ../../lib/decoder/PayloadDecoder.cpp
)

if (BUILD_FUNC_TESTS OR BUILD_UNIT_TEST)
  if (WIN32 AND CNAKE_TOOLCHAIN_FILE)
    find_package(GTest)
    if(NOT (GTEST_FOUND OR GTest_FOUND))
      install_windows_deps()
      if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        message(STATUS_FATAL "Windows dependency installation failed!")
      endif()
      message(STATUS ${CMAKE_TOOLCHAIN_FILE})
      find_package(GTest REQUIRED)
    endif()
  else()
    message("Adding gtest")
    add_library(gtest STATIC IMPORTED GLOBAL)
    message("Adding gmock")
    add_library(gmock STATIC IMPORTED GLOBAL)
    include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googletest/include
      ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest/googlemock/include
    )  
  endif()
endif()

if(BUILD_FUNC_TESTS)
  add_subdirectory(functests)
endif()
if(BUILD_UNIT_TESTS)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/unittests)
  add_subdirectory(unittests)
endif()

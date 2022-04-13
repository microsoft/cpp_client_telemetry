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
  if (WIN32 AND CMAKE_TOOLCHAIN_FILE)
    find_package(gtest REQUIRED)
    find_package(SQLite3 REQUIRED)
    find_package(ZLIB REQUIRED)

    # GMOCK is not included as part of GTEST package installed through vcpkg,
    #   so needs to be configured separately
    find_library(GOOGLE_MOCK gmock REQUIRED) 
    find_library(GOOGLE_MOCK_MAIN gmock_main REQUIRED)
    find_library(GOOGLE_MOCKD gmockd REQUIRED) 
    find_library(GOOGLE_MOCK_MAIND gmock_maind REQUIRED)

    # GMOCK doesn't work with static linking. 
    add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY)

    message("GTEST_INCLUDE_DIRS   = ${GTEST_INCLUDE_DIRS}")
    message("GTEST_BOTH_LIBRARIES = ${GTEST_BOTH_LIBRARIES}")
    message("GOOGLE_MOCK = ${GOOGLE_MOCK} ${GOOGLE_MOCK_MAIN}")
    message("SQLite3_LIBRARIES = ${SQLite3_LIBRARIES} ${SQLite3_INCLUDE_DIRS}")
    message("ZLIB_LIBRARIES = ${ZLIB_LIBRARIES}")

    #Include headers to be used both for unit and functional tests
    include_directories( ${GTEST_INCLUDE_DIRS} )
    include_directories( ${GMOCK_INCLUDE_DIRS} )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
    include_directories(${SQLite3_INCLUDE_DIRS} )
    
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

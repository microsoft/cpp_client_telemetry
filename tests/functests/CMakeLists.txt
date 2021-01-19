message("--- functests")

set(SRCS
  APITest.cpp
  BasicFuncTests.cpp
  LogSessionDataFuncTests.cpp
  Main.cpp
  MultipleLogManagersTests.cpp
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/ AND BUILD_PRIVACYGUARD)
  add_definitions(-DHAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
    PrivacyGuardFuncTests.cpp
  )
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests)
    list(APPEND SRCS
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSClientFuncTests.cpp
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSClientRealworldFuncTests.cpp
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSConfigCacheFuncTests.cpp
        )
endif()

source_group(" "      REGULAR_EXPRESSION "")
source_group("common" REGULAR_EXPRESSION "/tests/common/")

if(BUILD_IOS)
  add_library(FuncTests ${SRCS} ${TESTS_COMMON_SRCS})
else()
  add_executable(FuncTests ${SRCS} ${TESTS_COMMON_SRCS})
endif()

if(PAL_IMPLEMENTATION STREQUAL "WIN32")
  # Link against prebuilt libraries on Windows
  message("--- WIN32: Linking against prebuilt libraries")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/gtest")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/gmock") 
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/zlib")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/sqlite")
  # link_directories(${CMAKE_BINARY_DIR}/gtest/ ${CMAKE_BINARY_DIR}/gmock/ ${CMAKE_BINARY_DIR}/zlib/ ${CMAKE_BINARY_DIR}/sqlite/) 
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../zlib )
  target_link_libraries(FuncTests
  	mat
	wininet.lib
	${CMAKE_BINARY_DIR}/gtest/gtest.lib
	${CMAKE_BINARY_DIR}/gmock/gmock.lib
	${CMAKE_BINARY_DIR}/zlib/zlib.lib
	${CMAKE_BINARY_DIR}/sqlite/sqlite.lib
  )
else()

  # Prefer linking to more recent local sqlite3
  if(EXISTS "/usr/local/lib/libsqlite3.a")
    set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
  elseif(EXISTS "/usr/local/opt/sqlite/lib/libsqlite3.a")
    set (SQLITE3_LIB "/usr/local/opt/sqlite/lib/libsqlite3.a")
  else()
    set (SQLITE3_LIB "sqlite3")
  endif()

  # Find zlib
  find_package( ZLIB REQUIRED )
  include_directories( ${ZLIB_INCLUDE_DIRS} )

  set (PLATFORM_LIBS "")
  # Add flags for obtaining system UUID via IOKit
  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set (PLATFORM_LIBS "-framework CoreFoundation -framework Foundation")
    if(BUILD_IOS)
      set (PLATFORM_LIBS "${PLATFORM_LIBS} -framework UIKit -framework Network -framework SystemConfiguration")
    else()
      set (PLATFORM_LIBS "${PLATFORM_LIBS} -framework IOKit -framework Network -framework SystemConfiguration")
    endif()
  endif()

  # Raspberry Pi 4 with gcc-8 on ARMv7l requires -latomic
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set (PLATFORM_LIBS "atomic")
  endif()

  # Find libraries
  message("--- Linking libraries! ")
  message("Current Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  message("Binary Dir: ${CMAKE_BINARY_DIR}")

  find_file(LIBGTEST
    NAMES libgtest.a
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest/build/lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/googlemock/gtest/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/lib/
  )

  find_file(LIBGMOCK
    NAMES libgmock.a
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest/build/lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/googlemock/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/lib/
  )

  target_link_libraries(FuncTests 
    ${LIBGTEST}
    ${LIBGMOCK}
    mat
    ${ZLIB_LIBRARIES}
    ${SQLITE3_LIB}
    ${PLATFORM_LIBS}
    dl)

  if(NOT BUILD_IOS)
    target_link_libraries(FuncTests curl)
  endif()

endif()

add_test(FuncTests FuncTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/FuncTests.xml")

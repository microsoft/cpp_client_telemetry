message(STATUS "Building functests")

set(SRCS
  APITest.cpp
  BasicFuncTests.cpp
  LogSessionDataFuncTests.cpp
  Main.cpp
  MultipleLogManagersTests.cpp
)

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/" AND BUILD_PRIVACYGUARD)
  add_definitions(-DHAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
    "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/functests/PrivacyGuardFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/" AND BUILD_SANITIZER)
  list(APPEND SRCS
    "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/functests/SanitizerFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/")
  list(APPEND SRCS
     "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/tests/functests/DefaultDataViewerFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/liveeventinspector/" AND BUILD_LIVEEVENTINSPECTOR)
  add_definitions(-DHAVE_MAT_LIVEEVENTINSPECTOR)
  list(APPEND SRCS
    "${PROJECT_SOURCE_DIR}/lib/modules/liveeventinspector/tests/functests/LiveEventInspectorFuncTests.cpp"
  )
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests")
    list(APPEND SRCS
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSClientFuncTests.cpp"
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSClientRealworldFuncTests.cpp"
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/functests/ECSConfigCacheFuncTests.cpp"
        )
    if (EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/functests/test.json")
      configure_file("${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/functests/test.json" "${PROJECT_BINARY_DIR}/test.json" COPYONLY)
    endif()
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
  message(STATUS "WIN32: Linking against prebuilt libraries")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/gtest")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/gmock")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/zlib")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/sqlite")
  # link_directories(${PROJECT_BINARY_DIR}/gtest/ ${PROJECT_BINARY_DIR}/gmock/ ${PROJECT_BINARY_DIR}/zlib/ ${PROJECT_BINARY_DIR}/sqlite/)
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../zlib )
  target_link_libraries(FuncTests
  	mat
	wininet.lib
	${PROJECT_BINARY_DIR}/gtest/gtest.lib
	${PROJECT_BINARY_DIR}/gmock/gmock.lib
	${PROJECT_BINARY_DIR}/zlib/zlib.lib
	${PROJECT_BINARY_DIR}/sqlite/sqlite.lib
  )
else()

  # Prefer the SDK's bundled sqlite3 when present (e.g. the Android legacy
  # build, where the NDK has no system sqlite3), then a more recent local
  # sqlite3, otherwise the system library.
  if(TARGET sqlite3_bundled)
    set (SQLITE3_LIB sqlite3_bundled)
  elseif(EXISTS "/usr/local/lib/libsqlite3.a")
    set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
  elseif(EXISTS "/usr/local/opt/sqlite/lib/libsqlite3.a")
    set (SQLITE3_LIB "/usr/local/opt/sqlite/lib/libsqlite3.a")
  else()
    set (SQLITE3_LIB "sqlite3")
  endif()

  if(TARGET zlib_bundled)
    set(MATSDK_TEST_ZLIB zlib_bundled)
  else()
    find_package( ZLIB REQUIRED )
    set(MATSDK_TEST_ZLIB ZLIB::ZLIB)
    include_directories( ${ZLIB_INCLUDE_DIRS} )
  endif()

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

  message(STATUS "Linking libraries")
  message(STATUS "Current Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  message(STATUS "Binary Dir: ${PROJECT_BINARY_DIR}")

  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

  find_file(LIBGTEST
    NAMES libgtest.a
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest/build/lib/
  )

  find_file(LIBGMOCK
    NAMES libgmock.a
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest/build/lib/
  )

  target_link_libraries(FuncTests 
    ${LIBGTEST}
    ${LIBGMOCK}
    mat
    ${MATSDK_TEST_ZLIB}
    ${SQLITE3_LIB}
    ${PLATFORM_LIBS}
    dl)

  # Link curl only when the SDK actually uses the curl HTTP client (Linux, and
  # macOS without Apple HTTP). The tests don't use curl directly, and on the
  # Android legacy path mat uses HttpClient_Android (no system curl in the NDK).
  # MATSDK_NEEDS_CURL already excludes iOS/Apple-HTTP; use the CURL::libcurl
  # imported target so this is also correct under vcpkg (matches ZLIB::ZLIB above).
  if(MATSDK_NEEDS_CURL)
    target_link_libraries(FuncTests CURL::libcurl)
  endif()

endif()

add_test(FuncTests FuncTests "--gtest_output=xml:${PROJECT_BINARY_DIR}/test-reports/FuncTests.xml")

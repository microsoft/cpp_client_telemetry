message("--- unittests")

set(SRCS
  BackoffTests_ExponentialWithJitter.cpp
  BondSplicerTests.cpp
  ClockSkewManagerTests.cpp
  ContextFieldsProviderTests.cpp
  ControlPlaneProviderTests.cpp
  CorrelationVectorTests.cpp
  DebugEventSourceTests.cpp
  DiskLocalStorageTests.cpp
  EventFilterRegulatorTests.cpp
  EventFilterTests.cpp
  EventPropertiesStorageTests.cpp
  EventPropertiesTests.cpp
  GuidTests.cpp
  HttpClientCAPITests.cpp
  HttpClientManagerTests.cpp
  HttpClientTests.cpp
  HttpDeflateCompressionTests.cpp
  HttpRequestEncoderTests.cpp
  HttpResponseDecoderTests.cpp
  HttpServerTests.cpp
  LogManagerImplTests.cpp
  LogSessionDataTests.cpp
  Main.cpp
  MemoryStorageTests.cpp
  MetaStatsTests.cpp
  OacrTests.cpp
  OfflineStorageTests.cpp
  OfflineStorageTests_SQLite.cpp
  PackagerTests.cpp
  PalTests.cpp
  RouteTests.cpp
  StringUtilsTests.cpp
  TaskDispatcherCAPITests.cpp
  TransmissionPolicyManagerTests.cpp
  UtilsTests.cpp
)

source_group(" "      REGULAR_EXPRESSION "")
source_group("common" REGULAR_EXPRESSION "/tests/common/")

add_executable(UnitTests ${SRCS} ${TESTS_COMMON_SRCS})

if(PAL_IMPLEMENTATION STREQUAL "WIN32")
  # Link against prebuilt libraries on Windows
  message("--- WIN32: Linking against prebuilt libraries")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/gtest")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/gmock")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/zlib")
  message("--- WIN32: ... ${CMAKE_BINARY_DIR}/sqlite")
  # link_directories(${CMAKE_BINARY_DIR}/gtest/ ${CMAKE_BINARY_DIR}/gmock/ ${CMAKE_BINARY_DIR}/zlib/ ${CMAKE_BINARY_DIR}/sqlite/)
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../zlib )
  target_link_libraries(UnitTests
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
    set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit")
  endif()

  # Find libraries
  message("--- Linking libraries! ")
  message("Current Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  message("Binary Dir: ${CMAKE_BINARY_DIR}")

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/ )

  target_link_libraries(UnitTests 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/googlemock/gtest/libgtest.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../../googletest/build/googlemock/libgmock.a
    mat
    curl
    ${ZLIB_LIBRARIES}
    ${SQLITE3_LIB}
    ${PLATFORM_LIBS}
    dl)


endif()

add_test(UnitTests UnitTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/UnitTests.xml")

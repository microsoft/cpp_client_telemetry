message("--- unittests")

set(SRCS
  AIJsonSerializerTests.cpp
  AITelemetrySystemTests.cpp
  BackoffTests_ExponentialWithJitter.cpp
  BondSplicerTests.cpp
  ClockSkewManagerTests.cpp
  ContextFieldsProviderTests.cpp
  ControlPlaneProviderTests.cpp
  CorrelationVectorTests.cpp
  DebugEventSourceTests.cpp
  DeviceStateHandlerTests.cpp
  DiskLocalStorageTests.cpp
  EventFilterCollectionTests.cpp
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
  LoggerTests.cpp
  LogManagerImplTests.cpp
  LogSessionDataTests.cpp
  LogSessionDataDBTests.cpp
  Main.cpp
  MemoryStorageTests.cpp
  MetaStatsTests.cpp
  OacrTests.cpp
  OfflineStorageTests.cpp
  OfflineStorageTests_Room.cpp
  OfflineStorageTests_SQLite.cpp
  PackagerTests.cpp
  PalTests.cpp
  RouteTests.cpp
  StringUtilsTests.cpp
  TaskDispatcherCAPITests.cpp
  TransmissionPolicyManagerTests.cpp
  TransmitProfileRuleTests.cpp
  TransmitProfilesTests.cpp
  UtilsTests.cpp
  ZlibUtilsTests.cpp
)

if (APPLE)
  if (BUILD_IOS)
    list(APPEND SRCS SysInfoUtilsTests_iOS.cpp)
  else()
    list(APPEND SRCS SysInfoUtilsTests_Mac.cpp)
  endif()
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests)
    list(APPEND SRCS
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSConfigCacheTests.cpp 
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSClientUtilsTests.cpp
        ${CMAKE_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSClientTests.cpp
        )
endif()
        
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/ AND BUILD_PRIVACYGUARD)
  add_definitions(-DHAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
     ${CMAKE_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyGuardTests.cpp
  )
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/lib/modules/dataviewer/)
  list(APPEND SRCS
     ${CMAKE_SOURCE_DIR}/lib/modules/dataviewer/tests/unittests/DefaultDataViewerTests.cpp
     DataViewerCollectionTests.cpp
  )
endif()

source_group(" "      REGULAR_EXPRESSION "")
source_group("common" REGULAR_EXPRESSION "/tests/common/")

if(BUILD_IOS)
  add_library(UnitTests STATIC ${SRCS} ${TESTS_COMMON_SRCS})
else()
  add_executable(UnitTests ${SRCS} ${TESTS_COMMON_SRCS})
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
    set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit -framework SystemConfiguration -framework Foundation -framework Network")
    if(BUILD_IOS)
      set (PLATFORM_LIBS "${PLATFORM_LIBS} -framework UIKit")
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

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/ )

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

  target_link_libraries(UnitTests 
    ${LIBGTEST}
    ${LIBGMOCK}
    mat
    ${ZLIB_LIBRARIES}
    ${SQLITE3_LIB}
    ${PLATFORM_LIBS}
    dl)

  if(NOT BUILD_IOS)
    target_link_libraries(UnitTests curl)
  endif()

endif()

add_test(UnitTests UnitTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/UnitTests.xml")

message(STATUS "Building unittests")

set(SRCS
  AIJsonSerializerTests.cpp
  AITelemetrySystemTests.cpp
  AnnexKTests.cpp
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
  EventPropertiesDecoratorTests.cpp
  EventPropertiesStorageTests.cpp
  EventPropertiesTests.cpp
  GuidTests.cpp
  HttpClientCAPITests.cpp
  HttpClientCurlTests.cpp
  HttpClientManagerTests.cpp
  HttpClientTests.cpp
  HttpDeflateCompressionTests.cpp
  HttpRequestEncoderTests.cpp
  HttpResponseDecoderTests.cpp
  HttpServerTests.cpp
  InformationProviderImplTests.cpp
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

set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)

# Enable Azure Monitor unit tests when the module is present.
# The AIJsonSerializer test sources are guarded by HAVE_MAT_AI.
if (EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/azmon/AIJsonSerializer.hpp")
  add_definitions(-DHAVE_MAT_AI)
endif()

if (APPLE)
  if (BUILD_IOS)
    list(APPEND SRCS SysInfoUtilsTests_iOS.cpp)
  else()
    list(APPEND SRCS SysInfoUtilsTests_Mac.cpp)
  endif()
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests")
    list(APPEND SRCS
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSConfigCacheTests.cpp"
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSClientUtilsTests.cpp"
        "${PROJECT_SOURCE_DIR}/lib/modules/exp/tests/unittests/ECSClientTests.cpp"
        )
endif()
        
if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/" AND BUILD_PRIVACYGUARD)
  add_definitions(-DHAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/InitializationConfigurationTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyConcernEventTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyConcernMetadataProviderTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyGuardTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/SystematicSamplerTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/" AND BUILD_SANITIZER)
  list(APPEND SRCS
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerJwtTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerProviderTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerSitePathTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerStringUtilsTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerUrlTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SanitizerTrieTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/unittests/SPOPasswordTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/")
  list(APPEND SRCS
     "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/tests/unittests/DefaultDataViewerTests.cpp"
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
  message(STATUS "WIN32: Linking against prebuilt libraries")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/gtest")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/gmock")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/zlib")
  message(STATUS "WIN32: ... ${PROJECT_BINARY_DIR}/sqlite")
  # link_directories(${PROJECT_BINARY_DIR}/gtest/ ${PROJECT_BINARY_DIR}/gmock/ ${PROJECT_BINARY_DIR}/zlib/ ${PROJECT_BINARY_DIR}/sqlite/)
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../zlib )
  target_link_libraries(UnitTests
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
  elseif(EXISTS "/opt/homebrew/opt/sqlite/lib/libsqlite3.a")
    # Apple Silicon homebrew installs to /opt/homebrew instead of /usr/local
    set (SQLITE3_LIB "/opt/homebrew/opt/sqlite/lib/libsqlite3.a")
  else()
    find_package(SQLite3 REQUIRED)
    set (SQLITE3_LIB SQLite::SQLite3)
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
    set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit -framework SystemConfiguration -framework Foundation -framework Network")
    if(BUILD_IOS)
      set (PLATFORM_LIBS "${PLATFORM_LIBS} -framework UIKit")
    endif()
  endif()

  # Raspberry Pi 4 with gcc-8 on ARMv7l requires -latomic
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set (PLATFORM_LIBS "atomic")
  endif()

  message(STATUS "Linking libraries")
  message(STATUS "Current Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  message(STATUS "Binary Dir: ${PROJECT_BINARY_DIR}")

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/ )

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

  message(STATUS "GTEST: ${LIBGTEST}")
  message(STATUS "GMOCK: ${LIBGMOCK}")

  target_link_libraries(UnitTests 
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
  # MATSDK_NEEDS_CURL already excludes iOS/Apple-HTTP. Prefer the CURL::libcurl
  # imported target (correct under vcpkg, matches ZLIB::ZLIB above) and fall back
  # to the find-module variables on CMake < 3.12, which does not define it.
  if(MATSDK_NEEDS_CURL)
    if(TARGET CURL::libcurl)
      target_link_libraries(UnitTests CURL::libcurl)
    else()
      target_link_libraries(UnitTests ${CURL_LIBRARIES})
    endif()
  endif()

endif()

add_test(UnitTests UnitTests "--gtest_output=xml:${PROJECT_BINARY_DIR}/test-reports/UnitTests.xml")

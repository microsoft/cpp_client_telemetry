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
  KillSwitchManagerTests.cpp
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
  PayloadDecoderTests.cpp
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
if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "NONE")
  list(REMOVE_ITEM SRCS OfflineStorageTests_SQLite.cpp)
endif()
set(MATSDK_UNIT_TEST_DEFINITIONS)

if(NOT MSVC)
  set_source_files_properties(${SRCS}
    PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

# Enable Azure Monitor unit tests when the module is present.
# The AIJsonSerializer test sources are guarded by HAVE_MAT_AI.
if (EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/azmon/AIJsonSerializer.hpp")
  list(APPEND MATSDK_UNIT_TEST_DEFINITIONS HAVE_MAT_AI)
endif()

if (APPLE)
  if (MATSDK_PLATFORM_IOS)
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
        
if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/" AND MATSDK_BUILD_PRIVACYGUARD)
  list(APPEND MATSDK_UNIT_TEST_DEFINITIONS HAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/InitializationConfigurationTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyConcernEventTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyConcernMetadataProviderTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/PrivacyGuardTests.cpp"
  "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/unittests/SystematicSamplerTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/" AND MATSDK_BUILD_SANITIZER)
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

if(MATSDK_PLATFORM_IOS)
  add_library(UnitTests STATIC ${SRCS} ${TESTS_COMMON_SRCS})
else()
  add_executable(UnitTests ${SRCS} ${TESTS_COMMON_SRCS})
endif()
target_link_libraries(UnitTests PRIVATE matsdk_test_config)
if(MATSDK_UNIT_TEST_DEFINITIONS)
  target_compile_definitions(UnitTests PRIVATE ${MATSDK_UNIT_TEST_DEFINITIONS})
endif()

target_link_libraries(UnitTests PRIVATE
  mat
  ZLIB::ZLIB
  gtest
  gmock)
if(NOT MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "NONE")
  target_link_libraries(UnitTests PRIVATE SQLite::SQLite3)
endif()
if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(UnitTests PRIVATE nlohmann_json::nlohmann_json)
endif()

add_test(UnitTests UnitTests "--gtest_output=xml:${PROJECT_BINARY_DIR}/test-reports/UnitTests.xml")

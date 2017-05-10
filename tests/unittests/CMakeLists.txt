if(PAL_IMPLEMENTATION STREQUAL "Skype")
  set(CONFIG_SRCS
    RuntimeConfigTests_ECS.cpp
  )
endif()

set(SRCS
  BondSplicerTests.cpp
  ContextFieldsProviderTests.cpp
  EventPropertiesTests.cpp
  BackoffTests_ExponentialWithJitter.cpp
  HttpClientTests.cpp
  HttpClientManagerTests.cpp
  HttpDeflateCompressionTests.cpp
  HttpRequestEncoderTests.cpp
  HttpResponseDecoderTests.cpp
  HttpServerTests.cpp
  LoggerTests.cpp
  Main.cpp
  MetaStatsTests.cpp
  OfflineStorageTests.cpp
  OfflineStorageTests_SQLite.cpp
  OfflineStorageTests_SQLiteWithMock.cpp
  PackagerTests.cpp
  PalTests.cpp
  RouteTests.cpp
  TransmissionPolicyManagerTests.cpp
  UtilsTests.cpp
  ${CONFIG_SRCS}
)

source_group(" "      REGULAR_EXPRESSION "")
source_group("common" REGULAR_EXPRESSION "/tests/common/")

add_executable(UnitTests ${SRCS} ${TESTS_COMMON_SRCS})
target_link_libraries(UnitTests gtest gmock aria)
add_test(UnitTests UnitTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/UnitTests.xml")

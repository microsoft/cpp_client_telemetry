message(STATUS "Building functests")

set(SRCS
  APITest.cpp
  BasicFuncTests.cpp
  LogSessionDataFuncTests.cpp
  Main.cpp
  MultipleLogManagersTests.cpp
)
set(MATSDK_FUNC_TEST_DEFINITIONS)

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/" AND MATSDK_BUILD_PRIVACYGUARD)
  list(APPEND MATSDK_FUNC_TEST_DEFINITIONS HAVE_MAT_PRIVACYGUARD)
  list(APPEND SRCS
    "${PROJECT_SOURCE_DIR}/lib/modules/privacyguard/tests/functests/PrivacyGuardFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/" AND MATSDK_BUILD_SANITIZER)
  list(APPEND SRCS
    "${PROJECT_SOURCE_DIR}/lib/modules/sanitizer/tests/functests/SanitizerFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/")
  list(APPEND SRCS
     "${PROJECT_SOURCE_DIR}/lib/modules/dataviewer/tests/functests/DefaultDataViewerFuncTests.cpp"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/lib/modules/liveeventinspector/" AND MATSDK_BUILD_LIVEEVENTINSPECTOR)
  list(APPEND MATSDK_FUNC_TEST_DEFINITIONS HAVE_MAT_LIVEEVENTINSPECTOR)
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

if(MATSDK_PLATFORM_IOS)
  add_library(FuncTests ${SRCS} ${TESTS_COMMON_SRCS})
else()
  add_executable(FuncTests ${SRCS} ${TESTS_COMMON_SRCS})
endif()
target_link_libraries(FuncTests PRIVATE matsdk_test_config)
if(MATSDK_FUNC_TEST_DEFINITIONS)
  target_compile_definitions(FuncTests PRIVATE ${MATSDK_FUNC_TEST_DEFINITIONS})
endif()

target_link_libraries(FuncTests PRIVATE
  mat
  ZLIB::ZLIB
  gtest
  gmock)
if(NOT MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "NONE")
  target_link_libraries(FuncTests PRIVATE SQLite::SQLite3)
endif()
if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(FuncTests PRIVATE nlohmann_json::nlohmann_json)
endif()

add_test(FuncTests FuncTests "--gtest_output=xml:${PROJECT_BINARY_DIR}/test-reports/FuncTests.xml")

if(PAL_IMPLEMENTATION STREQUAL "Skype")
  set(PAL_SPECIFIC_SRCS
    SkypeResourceManagerTests.cpp
  )
endif()

set(SRCS
  BasicFuncTests.cpp
  LoadTests.cpp
  Main.cpp
  MultipleLogManagersTests.cpp
  ${PAL_SPECIFIC_SRCS}
)

source_group(" "      REGULAR_EXPRESSION "")
source_group("common" REGULAR_EXPRESSION "/tests/common/")

add_executable(FuncTests ${SRCS} ${TESTS_COMMON_SRCS})
target_link_libraries(FuncTests gtest gmock aria)
add_test(FuncTests FuncTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/FuncTests.xml")

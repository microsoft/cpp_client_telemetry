include_directories(../include)
include_directories(../generated)

add_executable(BondLiteUnitTests
  BondLiteTests.cpp
  CompactBinaryProtocolTests.cpp
  FullDumpBinaryBlob.hpp
  Main.cpp
)
source_group(" " REGULAR_EXPRESSION "")
target_link_libraries(BondLiteUnitTests gtest gmock)
add_test(BondLiteUnitTests BondLiteUnitTests "--gtest_output=xml:${CMAKE_BINARY_DIR}/test-reports/BondLiteUnitTests.xml")


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../external/bond)
  set(BOOST_INCLUDE_PATH "" CACHE PATH "Path to Boost include directory (where 'boost' subdirectory is)")
  if(NOT EXISTS ${BOOST_INCLUDE_PATH}/boost/version.hpp)
    message(FATAL_ERROR "bondlite/tests: Full Bond detected in bondlite/external, but Boost not found. Set BOOST_INCLUDE_PATH.")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../external/rapidjson)
    message(FATAL_ERROR "bondlite/tests: Full Bond detected in bondlite/external, but RapidJSON not found. Copy 'rapidjson' includes to bondlite/external too.")
  endif()

  message(STATUS "bondlite/tests: Boost & full Bond detected, will test against the reference implementation.")
  include_directories(../external)
  include_directories(${BOOST_INCLUDE_PATH})
  add_definitions(-DHAS_BOOST_AND_FULL_BOND=1)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DBOOST_ALL_NO_LIB -DBOND_COMPACT_BINARY_PROTOCOL)
  set_target_properties(BondLiteUnitTests PROPERTIES
    CFW_CXX_EXCEPTIONS ON
    CFW_CXX_RTTI       ON
  )
  target_sources(BondLiteUnitTests PRIVATE
    ../generated/bond-aria/DataPackage_apply.cpp
    ../generated/bond-aria/DataPackage_types.cpp
  )
else()
  message(STATUS "bondlite/tests: Boost & full Bond not detected, will test against hard-coded values only.")
endif()


add_executable(BondLiteDump
  BondLiteDump.cpp
)

include_directories(.)
include_directories(../lib)

if(PAL_IMPLEMENTATION STREQUAL "Skype")
  set(TESTS_COMMON_SKYPE_SRCS
    ../common/MockIEcsClient.hpp
  )
endif()

set(TESTS_COMMON_SRCS
  ../common/Common.cpp
  ../common/Common.hpp
  ../common/HttpServer.hpp
  ../common/Mocks.cpp
  ../common/MockIHttpClient.hpp
  ../common/MockILogManagerInternal.hpp
  ../common/MockIOfflineStorage.hpp
  ../common/MockIOfflineStorageObserver.hpp
  ../common/MockIRuntimeConfig.hpp
  ../common/MockISemanticContext.hpp
  ../common/MockISqlite3Proxy.hpp
  ../common/SocketTools.hpp
  ${TESTS_COMMON_SKYPE_SRCS}
)

if(BUILD_UNIT_TESTS)
  add_subdirectory(unittests)
endif()

if(BUILD_FUNC_TESTS)
  add_subdirectory(functests)
endif()

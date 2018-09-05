include_directories( . ${CMAKE_CURRENT_SOURCE_DIR}/include/public ${CMAKE_CURRENT_SOURCE_DIR}/include/public ${CMAKE_CURRENT_SOURCE_DIR}/include/aria ${CMAKE_CURRENT_SOURCE_DIR}/pal ${CMAKE_CURRENT_SOURCE_DIR}/utils /usr/local/include )

set(SRCS decorators/BaseDecorator.cpp
  packager/BondSplicer.cpp
  packager/Packager.cpp
  callbacks/DebugSource.cpp
  bond/BondSerializer.cpp
  filter/EventFilterRegulator.cpp
  filter/EventFilter.cpp
  tpm/TransmitProfiles.cpp
  tpm/TransmissionPolicyManager.cpp
  tpm/DeviceStateHandler.cpp
  system/EventProperty.cpp
  system/TelemetrySystem.cpp
  system/EventProperties.cpp
  compression/HttpDeflateCompression.cpp
  api/LogManager.cpp
  api/ContextFieldsProvider.cpp
  api/LogManagerImpl.cpp
  api/LogSessionData.cpp
  api/Logger.cpp
  api/LogManagerProvider.cpp
  api/CorrelationVector.cpp
  api/LogConfiguration.cpp
  api/AuthTokensController.cpp
  api/LogManagerFactory.cpp
  utils/Utils.cpp
  utils/StringUtils.cpp
  exp/afd/afdclient/AFDClientUtils.cpp
  exp/afd/afdclient/AFDClient.cpp
  exp/afd/afdclient/AFDConfigCache.cpp
  exp/ecs/ecsclient/ECSClient.cpp
  exp/ecs/ecsclient/ECSClientUtils.cpp
  exp/ecs/ecsclient/ECSConfigCache.cpp
  exp/JsonHelper.cpp
  exp/EXPCommonClient.cpp
  pal/NetworkInformationImpl.cpp
  pal/DeviceInformationImpl.cpp
  pal/InformationProviderImpl.cpp
  http/HttpClientManager.cpp
  http/HttpRequestEncoder.cpp
  http/HttpResponseDecoder.cpp
  stats/Statistics.cpp
  stats/MetaStats.cpp
  offline/StorageObserver.cpp
  offline/MemoryStorage.cpp
  offline/OfflineStorage_SQLite.cpp
  offline/FifoFileStorage.cpp
  offline/OfflineStorageHandler.cpp
  backoff/IBackoff.cpp
)

if(PAL_IMPLEMENTATION STREQUAL "CPP11")
  list(APPEND SRCS
    http/HttpClient_Curl.cpp
    http/HttpClient_Curl.hpp
    http/HttpClient.hpp
    pal/PAL_CPP11.cpp
    pal/PAL_CPP11.hpp
    pal/linux/LinuxDeviceInformationImpl.cpp
    pal/linux/LinuxNetworkInformationImpl.cpp
    pal/linux/LinuxSystemInformationImpl.cpp
    pal/linux/sysinfo_sources.cpp
  )
else()
  message(FATAL_ERROR "No platform abstraction library configured. Set PAL_IMPLEMENTATION.")
endif()

if(ARIASDK_UTC_ENABLED)
  list(APPEND SRCS
    utc/MicrosoftTelemetry.h
    utc/traceloggingdynamic.h
    utc/UtcForwarder.cpp
    utc/UtcForwarder.hpp
  )
endif()

create_source_files_groups_per_folder(${SRCS})

# MinGW does not require pthread
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(Threads REQUIRED)
  if(THREADS_HAVE_PTHREAD_ARG)
   target_compile_options(aria PUBLIC "-pthread")
  endif()
endif()

if(BUILD_SHARED_LIBS STREQUAL "ON")
  message("-- Building shared library")
  add_library(aria SHARED ${SRCS})
  add_library(sqlite3 SHARED IMPORTED GLOBAL)
  add_library(z SHARED IMPORTED GLOBAL)
  #target_link_libraries(aria PUBLIC libsqlite3.so libz.so ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" "${Tcmalloc_LIBRARIES}")
  target_link_libraries(aria PUBLIC libsqlite3.so libz.so ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}")
  # target_link_libraries(aria PUBLIC libsqlite3 libcurl.a libz.a libssl.a libcrypto.a "${SQLITE_LIBRARY}" "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
  install(TARGETS aria EXPORT aria LIBRARY DESTINATION ${INSTALL_LIB_DIR})
else()
  message("-- Building static library")
  add_library(aria STATIC ${SRCS})
  add_library(sqlite3 STATIC IMPORTED GLOBAL)
  add_library(z STATIC IMPORTED GLOBAL)
  #target_link_libraries(aria PRIVATE libsqlite3.a PUBLIC libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" "${Tcmalloc_LIBRARIES}")
  target_link_libraries(aria PRIVATE libsqlite3.a PUBLIC libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
  install(TARGETS aria EXPORT aria ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
endif()

message("-- Library will be installed to ${INSTALL_LIB_DIR}")

#if(PAL_IMPLEMENTATION STREQUAL "CPP11")
#  #target_link_libraries(aria PUBLIC libcurl.a libz.a libssl.a libcrypto.a "${SQLITE_LIBRARY}" "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
#  #target_link_libraries(aria PUBLIC libsqlite3.a libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
#endif()


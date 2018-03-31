include_directories( . ${CMAKE_CURRENT_SOURCE_DIR}/include/public ${CMAKE_CURRENT_SOURCE_DIR}/include/public ${CMAKE_CURRENT_SOURCE_DIR}/include/msevents ${CMAKE_CURRENT_SOURCE_DIR}/pal ${CMAKE_CURRENT_SOURCE_DIR}/utils /usr/local/include )

if(PAL_IMPLEMENTATION STREQUAL "CPP11")
  set(PAL_SRCS
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

set(SRCS

  api/AuthTokensController.cpp
  api/CommonLogManagerInternal.cpp
  api/ContextFieldsProvider.cpp
  api/CorrelationVector.cpp
  api/HostGuestLogManager.cpp
  api/LogConfiguration.cpp
  api/LogController.cpp
  api/LogManager.cpp
  api/LogManagerImpl.cpp
  api/LogManagerProvider.cpp
  api/LogSessionData.cpp
  api/Logger.cpp

  backoff/IBackoff.cpp
  bond/BondSerializer.cpp
  callbacks/DebugSource.cpp

  compression/HttpDeflateCompression.cpp
  config/RuntimeConfig_Default.cpp
  
  include/public/LogManager.hpp
  utils/Utils.cpp
  utils/StringUtils.cpp

  backoff/Backoff_ExponentialWithJitter.hpp
  backoff/IBackoff.cpp
  backoff/IBackoff.hpp

  bond/generated/AriaProtocol_readers.hpp
  bond/generated/AriaProtocol_types.hpp
  bond/generated/AriaProtocol_writers.hpp
  bond/generated/BondConstTypes.hpp
  bond/BondSerializer.cpp
  bond/BondSerializer.hpp

  compression/HttpDeflateCompression.cpp
  compression/HttpDeflateCompression.hpp

  config/RuntimeConfig_Default.cpp
  config/RuntimeConfig_Default.hpp

  decorators/BaseDecorator.hpp
  decorators/EventPropertiesDecorator.hpp
  decorators/IDecorator.hpp
  decorators/RuntimeConfigDecorator.hpp
  decorators/SemanticApiDecorators.hpp
  decorators/SemanticContextDecorator.hpp

  http/HttpClientManager.cpp
  http/HttpClientManager.hpp
  http/HttpRequestEncoder.cpp
  http/HttpRequestEncoder.hpp
  http/HttpResponseDecoder.cpp
  http/HttpResponseDecoder.hpp

  include/public/ctmacros.hpp
  include/public/Enums.hpp
  include/public/EventProperties.hpp
  include/public/IBandwidthController.hpp
  include/public/IHttpClient.hpp
  include/public/ILogger.hpp
  include/public/ILogManager.hpp

  include/msevents/IOfflineStorage.hpp
  include/msevents/ISystemInformation.hpp

  include/public/IRuntimeConfig.hpp
  include/public/ISemanticContext.hpp
#  include/public/LogConfiguration.hpp
  utils/Utils.hpp
  include/public/Version.hpp

  ${JNI_SRCS}

  # TODO: [MG] - check if ClockSkewManager.cpp is needed
  # offline/ClockSkewManager.cpp
  offline/FifoFileStorage.cpp
  offline/OfflineStorage.cpp
  offline/OfflineStorageHandler.cpp
  offline/OfflineStorage_SQLite.cpp

  packager/BondSplicer.cpp
  packager/BondSplicer.hpp
  packager/Packager.cpp
  packager/Packager.hpp

  pal/PAL.hpp
  pal/InformationProviderImpl.cpp
  pal/DeviceInformationImpl.cpp
  pal/NetworkInformationImpl.cpp
  ${PAL_SRCS}

  stats/MetaStats.cpp
  stats/MetaStats.hpp
  stats/Statistics.cpp
  stats/Statistics.hpp

  system/Contexts.hpp
  system/Route.hpp
  system/TelemetrySystem.cpp
  system/TelemetrySystem.hpp
  system/EventProperties.cpp
  system/EventProperty.cpp

  tpm/DeviceStateHandler.cpp
  tpm/TransmissionPolicyManager.cpp
  tpm/TransmissionPolicyManager.hpp
  tpm/TransmitProfiles.cpp
)

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
   target_compile_options(msevents PUBLIC "-pthread")
  endif()
endif()

if(BUILD_SHARED_LIBS STREQUAL "ON")
  message("-- Building shared library")
  add_library(msevents SHARED ${SRCS})
  add_library(sqlite3 SHARED IMPORTED GLOBAL)
  add_library(z SHARED IMPORTED GLOBAL)
  #target_link_libraries(msevents PUBLIC libsqlite3.so libz.so ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" "${Tcmalloc_LIBRARIES}")
  target_link_libraries(msevents PUBLIC libsqlite3.so libz.so ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}")
  # target_link_libraries(msevents PUBLIC libsqlite3 libcurl.a libz.a libssl.a libcrypto.a "${SQLITE_LIBRARY}" "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
  install(TARGETS msevents EXPORT msevents LIBRARY DESTINATION ${INSTALL_LIB_DIR})
else()
  message("-- Building static library")
  add_library(msevents STATIC ${SRCS})
  add_library(sqlite3 STATIC IMPORTED GLOBAL)
  add_library(z STATIC IMPORTED GLOBAL)
  #target_link_libraries(msevents PRIVATE libsqlite3.a PUBLIC libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" "${Tcmalloc_LIBRARIES}")
  target_link_libraries(msevents PRIVATE libsqlite3.a PUBLIC libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
  install(TARGETS msevents EXPORT msevents ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
endif()

message("-- Library will be installed to ${INSTALL_LIB_DIR}")

#if(PAL_IMPLEMENTATION STREQUAL "CPP11")
#  #target_link_libraries(msevents PUBLIC libcurl.a libz.a libssl.a libcrypto.a "${SQLITE_LIBRARY}" "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
#  #target_link_libraries(msevents PUBLIC libsqlite3.a libz.a ${LIBS} "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}" )
#endif()


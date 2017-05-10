include_directories(.)

if(PAL_IMPLEMENTATION STREQUAL "Skype")
  set(PAL_SRCS
    bwcontrol/BandwidthController_ResourceManager.cpp
    bwcontrol/BandwidthController_ResourceManager.hpp
    config/RuntimeConfig_ECS.cpp
    config/RuntimeConfig_ECS.hpp
    http/HttpClient_HttpStack.cpp
    http/HttpClient_HttpStack.hpp
    pal/PAL_Skype.cpp
    pal/PAL_Skype.hpp
  )
elseif(PAL_IMPLEMENTATION STREQUAL "Win32")
  set(PAL_SRCS
    http/HttpClient_WinInet.cpp
    http/HttpClient_WinInet.hpp
    pal/PAL_Win32.cpp
    pal/PAL_Win32.hpp
  )
else()
  message(FATAL_ERROR "No platform abstraction library configured. Set PAL_IMPLEMENTATION.")
endif()

if(BUILD_JNI_WRAPPER)
  add_subdirectory(jni)

  include_directories(jni/gen)

  set(JNI_SRCS
    jni/AriaProxy.cpp
    jni/JavaUtils.cpp
    jni/JavaUtils.hpp
  )
endif()

set(SRCS
  api/ContextFieldsProvider.cpp
  api/ContextFieldsProvider.hpp
  api/Logger.cpp
  api/Logger.hpp
  api/LogManager.cpp
  api/LogManager.hpp
  api/Utils.cpp

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

  include/aria/AggregatedMetric.hpp
  include/aria/ctmacros.hpp
  include/aria/Enums.hpp
  include/aria/EventProperties.hpp
  include/aria/IBandwidthController.hpp
  include/aria/IHttpClient.hpp
  include/aria/ILogger.hpp
  include/aria/ILogManager.hpp
  include/aria/IOfflineStorage.hpp
  include/aria/IRuntimeConfig.hpp
  include/aria/ISemanticContext.hpp
  include/aria/LogConfiguration.hpp
  include/aria/Utils.hpp
  include/aria/Version.hpp
  include/aria/Version.hpp.template

  ${JNI_SRCS}

  offline/ISqlite3Proxy.hpp
  offline/OfflineStorage.cpp
  offline/OfflineStorage.hpp
  offline/OfflineStorage_SQLite.hpp
  offline/OfflineStorage_SQLite.cpp
  offline/SQLiteWrapper.hpp

  packager/BondSplicer.cpp
  packager/BondSplicer.hpp
  packager/Packager.cpp
  packager/Packager.hpp

  pal/PAL.hpp
  ${PAL_SRCS}

  stats/MetaStats.cpp
  stats/MetaStats.hpp
  stats/Statistics.cpp
  stats/Statistics.hpp

  system/Contexts.hpp
  system/Route.hpp
  system/TelemetrySystem.cpp
  system/TelemetrySystem.hpp

  tpm/TransmissionPolicyManager.cpp
  tpm/TransmissionPolicyManager.hpp

  utils/Common.cpp
  utils/Common.hpp
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
add_library(aria STATIC ${SRCS})
if(BUILD_JNI_WRAPPER)
  add_dependencies(aria aria_jni_wrapper)
endif()

if(PAL_IMPLEMENTATION STREQUAL "Skype")
  target_link_libraries(aria PUBLIC httpstack_lib sqlite_lib resourcemanager_lib json_lib auf_lib rtnet_lib)
  install(TARGETS aria EXPORT aria ARCHIVE DESTINATION lib)
  if(LOGMAP_ENABLED)
    add_library_logmap(aria NO_VERIFY)
    get_logmap_location(LOGMAP_PATH aria INSTALL)
    install(FILES ${LOGMAP_PATH} DESTINATION lib)
  endif()
elseif(PAL_IMPLEMENTATION STREQUAL "Win32")
  target_link_libraries(aria PUBLIC rpcrt4.lib wininet.lib iphlpapi.lib sqlite3 z)
  install(TARGETS aria EXPORT aria ARCHIVE DESTINATION lib)
endif()

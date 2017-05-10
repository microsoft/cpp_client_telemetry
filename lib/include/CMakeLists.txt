string(REPLACE "." ";" ARIASDK_BUILD_VERSION_SPLIT "${ARIASDK_BUILD_VERSION}")
list(GET ARIASDK_BUILD_VERSION_SPLIT 0 BUILD_VERSION_MAJOR)
list(GET ARIASDK_BUILD_VERSION_SPLIT 1 BUILD_VERSION_MINOR)
list(GET ARIASDK_BUILD_VERSION_SPLIT 2 BUILD_VERSION_PATCH)
list(GET ARIASDK_BUILD_VERSION_SPLIT 3 BUILD_NUMBER)

configure_file(aria/Version.hpp.template "${CMAKE_CURRENT_SOURCE_DIR}/aria/Version.hpp")

install(
  FILES
    aria/AggregatedMetric.hpp
    aria/ctmacros.hpp
    aria/Enums.hpp
    aria/EventProperties.hpp
    aria/IBandwidthController.hpp
    aria/IHttpClient.hpp
    aria/ILogger.hpp
    aria/ILogManager.hpp
    aria/IOfflineStorage.hpp
    aria/IRuntimeConfig.hpp
    aria/ISemanticContext.hpp
    aria/LogConfiguration.hpp
    aria/Utils.hpp
    aria/Version.hpp
  DESTINATION
    include/aria
)

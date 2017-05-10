# Install script for directory: C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/AriaSDK")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/aria" TYPE FILE FILES
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/AggregatedMetric.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/ctmacros.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/Enums.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/EventProperties.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/IBandwidthController.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/IHttpClient.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/ILogger.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/ILogManager.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/IOfflineStorage.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/IRuntimeConfig.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/ISemanticContext.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/LogConfiguration.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/Utils.hpp"
    "C:/ASGVSO/CPP1/Aria.SDK.C/Aria.SDK.NewC/lib/include/aria/Version.hpp"
    )
endif()


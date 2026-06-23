# Honor visibility properties for all target types
cmake_policy(SET CMP0063 NEW)

# Legacy (non-target) include paths that apply globally within this directory and
# are used by build.sh / MSBuild / standalone CMake workflows. They do NOT propagate
# to downstream consumers via find_package() (see target_include_directories below).
include_directories( . ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/public ${CMAKE_CURRENT_SOURCE_DIR}/include/mat ${CMAKE_CURRENT_SOURCE_DIR}/pal ${CMAKE_CURRENT_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR}/modules/exp ${CMAKE_CURRENT_SOURCE_DIR}/modules/dataviewer ${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard ${CMAKE_CURRENT_SOURCE_DIR}/modules/liveeventinspector ${CMAKE_CURRENT_SOURCE_DIR}/modules/cds ${CMAKE_CURRENT_SOURCE_DIR}/modules/signals ${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer )

# Legacy builds may need system-installed deps from /usr/local/include
if(NOT MATSDK_USE_VCPKG_DEPS)
  include_directories(/usr/local/include)
endif()

set(SRCS decorators/BaseDecorator.cpp
  packager/BondSplicer.cpp
  packager/Packager.cpp
  callbacks/DebugSource.cpp
  bond/BondSerializer.cpp
  filter/EventFilterCollection.cpp
  tpm/TransmitProfiles.cpp
  tpm/TransmissionPolicyManager.cpp
  tpm/DeviceStateHandler.cpp
  system/EventProperty.cpp
  system/TelemetrySystem.cpp
  system/EventProperties.cpp
  compression/HttpDeflateCompression.cpp
  api/AllowedLevelsCollection.cpp
  api/LogManager.cpp
  api/ContextFieldsProvider.cpp
  api/LogManagerImpl.cpp
  api/LogSessionData.cpp
  api/Logger.cpp
  api/LogManagerProvider.cpp
  api/CorrelationVector.cpp
  api/LogConfiguration.cpp
  api/AuthTokensController.cpp
  api/ILogConfiguration.cpp
  api/LogManagerFactory.cpp
  api/capi.cpp
  api/DataViewerCollection.cpp
  utils/FileUtils.cpp
  utils/Utils.cpp
  utils/StringUtils.cpp
  utils/ZlibUtils.cpp
  pal/InformationProviderImpl.cpp
  http/HttpClient_CAPI.cpp
  http/HttpClientManager.cpp
  http/HttpRequestEncoder.cpp
  http/HttpResponseDecoder.cpp
  http/HttpClientFactory.cpp
  stats/Statistics.cpp
  stats/MetaStats.cpp
  offline/StorageObserver.cpp
  offline/OfflineStorageFactory.cpp
  offline/MemoryStorage.cpp
  offline/OfflineStorage_SQLite.cpp
  offline/OfflineStorageHandler.cpp
  offline/LogSessionDataProvider.cpp
  backoff/IBackoff.cpp
  pal/PAL.cpp
  pal/TaskDispatcher_CAPI.cpp
  pal/WorkerThread.cpp
  decoder/PayloadDecoder.cpp
)

# Support for Azure Monitor / Application Insights
if(BUILD_AZMON)
  include(modules/azmon/CMakeLists.txt OPTIONAL)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/exp/")
  list(APPEND SRCS
    modules/exp/afd/afdclient/AFDClientUtils.cpp
    modules/exp/afd/afdclient/AFDClient.cpp
    modules/exp/afd/afdclient/AFDConfigCache.cpp
    modules/exp/ecs/ecsclient/ECSClient.cpp
    modules/exp/ecs/ecsclient/ECSClientUtils.cpp
    modules/exp/ecs/ecsclient/ECSConfigCache.cpp
    modules/exp/JsonHelper.cpp
    modules/exp/EXPCommonClient.cpp
    modules/exp/FifoFileStorage.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/dataviewer/")
  list(APPEND SRCS
    modules/dataviewer/DefaultDataViewer.cpp
    modules/dataviewer/OnDisableNotificationCollection.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/" AND BUILD_PRIVACYGUARD)
  list(APPEND SRCS
    modules/privacyguard/PrivacyGuard.cpp
    modules/privacyguard/RegisteredFileTypes.cpp
    modules/privacyguard/SummaryStatistics.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/liveeventinspector/" AND BUILD_LIVEEVENTINSPECTOR)
  list(APPEND SRCS
    modules/liveeventinspector/LiveEventInspector.cpp
    modules/liveeventinspector/LiveEventInspector.hpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/cds/" AND BUILD_CDS)
  add_definitions(-DHAVE_MAT_CDS)
  list(APPEND SRCS
    modules/cds/CdsFactory.hpp
    modules/cds/CdsFactory.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/signals/" AND BUILD_SIGNALS)
  list(APPEND SRCS
    modules/signals/Signals.cpp
    modules/signals/SignalsEncoder.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer/" AND BUILD_SANITIZER)
  list(APPEND SRCS
    modules/sanitizer/detectors/EmailAddressDetector.cpp
    modules/sanitizer/detectors/JwtDetector.cpp
    modules/sanitizer/detectors/SPOPassword.cpp
    modules/sanitizer/detectors/UrlDetector.cpp
    modules/sanitizer/Sanitizer.cpp
    modules/sanitizer/SanitizerProvider.cpp
    modules/sanitizer/SanitizerStringUtils.cpp
    modules/sanitizer/SanitizerTargets.cpp
    modules/sanitizer/SanitizerTrie.cpp
    modules/sanitizer/SanitizerTrieNode.cpp
  )
  # Suppress -Wreorder for Sanitizer.cpp only: the submodule declares
  # m_sanitizerprovider before m_notificationEventName in Sanitizer.hpp
  # but the constructor init list reverses that order.  The proper fix
  # belongs in the lib/modules submodule; this scopes the suppression
  # until that is done.
  if(NOT MSVC)
    set_source_files_properties(modules/sanitizer/Sanitizer.cpp
      PROPERTIES COMPILE_FLAGS "-Wno-reorder")
  endif()
endif()

set(MATSDK_OBJC_PRIVACYGUARD_AVAILABLE OFF)
set(MATSDK_OBJC_SANITIZER_AVAILABLE OFF)

if(PAL_IMPLEMENTATION STREQUAL "CPP11")
  if(APPLE)
    list(APPEND SRCS
      pal/posix/sysinfo_utils_apple.cpp
    )
    if(BUILD_IOS)
      list(APPEND SRCS
        pal/posix/sysinfo_utils_ios.mm
      )
    else()
      list(APPEND SRCS
        pal/posix/sysinfo_utils_mac.mm
      )
    endif()
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(APPEND SRCS
      pal/posix/DeviceInformationImpl_Android.cpp
      pal/posix/SystemInformationImpl_Android.cpp
      pal/posix/sysinfo_sources.cpp
    )
  else()
    list(APPEND SRCS
      pal/posix/DeviceInformationImpl.cpp
      pal/posix/SystemInformationImpl.cpp
      pal/posix/sysinfo_sources.cpp
    )
  endif()

  if(APPLE)
    if(BUILD_APPLE_HTTP OR BUILD_IOS)
      list(APPEND SRCS
        http/HttpClient_Apple.mm
      )
    else()
      list(APPEND SRCS
        http/HttpClient_Curl.cpp
        http/HttpClient_Curl.hpp
      )
    endif()

    list(APPEND SRCS
        pal/posix/NetworkInformationImpl.mm
    )
  else()
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
      list(APPEND SRCS
        pal/posix/NetworkInformationImpl_Android.cpp
      )
      if(MATSDK_USE_VCPKG_DEPS)
        list(APPEND SRCS
          http/HttpClient_Curl.cpp
          http/HttpClient_Curl.hpp
        )
      else()
        list(APPEND SRCS
          http/HttpClient_Android.cpp
          http/HttpClient_Android.hpp
        )
      endif()
    else()
      list(APPEND SRCS
        pal/posix/NetworkInformationImpl.cpp
        http/HttpClient_Curl.cpp
        http/HttpClient_Curl.hpp
      )
    endif()
  endif()
  if(APPLE AND BUILD_OBJC_WRAPPER)
    message(STATUS "Include ObjC Wrappers")
    list(APPEND SRCS
      ../wrappers/obj-c/ODWLogger.mm
      ../wrappers/obj-c/ODWLogManager.mm
      ../wrappers/obj-c/ODWEventProperties.mm
      ../wrappers/obj-c/ODWLogConfiguration.mm
      ../wrappers/obj-c/ODWSemanticContext.mm
      ../wrappers/obj-c/ODWCommonDataContext.mm
      ../wrappers/obj-c/ODWPrivacyGuardInitConfig.mm
      ../wrappers/obj-c/ODWSanitizerInitConfig.mm
      )
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/dataviewer/")
        list(APPEND SRCS
        ../wrappers/obj-c/ODWDiagnosticDataViewer.mm
        )
    endif()
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/" AND BUILD_PRIVACYGUARD)
          set(MATSDK_OBJC_PRIVACYGUARD_AVAILABLE ON)
          list(APPEND SRCS
          ../wrappers/obj-c/ODWPrivacyGuard.mm
            )
      endif()
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer/" AND BUILD_SANITIZER)
        set(MATSDK_OBJC_SANITIZER_AVAILABLE ON)
        list(APPEND SRCS
        ../wrappers/obj-c/ODWSanitizer.mm
        )
      endif()
  endif()

  if(APPLE AND BUILD_SWIFT_WRAPPER)
    message(STATUS "Building Swift Wrappers")
    # Run swift build for the Swift Wrappers Package
    string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_BUILD_TYPE)
    execute_process(
      COMMAND swift build
      -c ${LOWER_BUILD_TYPE}
      --scratch-path ${CMAKE_BINARY_DIR}/OneDSSwift
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../wrappers/swift
      RESULT_VARIABLE SWIFT_BUILD_RESULT
    )

    if(SWIFT_BUILD_RESULT EQUAL 0)
        message(STATUS "Swift Wrappers build succeeded!")
    else()
        message(FATAL_ERROR "Swift build failed with error code: ${SWIFT_BUILD_RESULT}")
    endif()

  endif()

elseif(PAL_IMPLEMENTATION STREQUAL "WIN32")
# Win32 Desktop for now.
# TODO: define a separate PAL for Win10 cmake build
if(NOT MATSDK_USE_VCPKG_DEPS)
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../zlib ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite)
endif()
add_definitions(-D_UNICODE -DUNICODE -DWIN32 -DMATSDK_PLATFORM_WINDOWS=1 -D_UTC_SDK -DUSE_BOND -D_WINDOWS -D_USRDLL -DWINVER=_WIN32_WINNT_WIN7)
remove_definitions(-D_MBCS)
  list(APPEND SRCS
    http/HttpClient_WinInet.cpp
    http/HttpClient_WinInet.hpp
    pal/desktop/WindowsDesktopDeviceInformationImpl.cpp
    pal/desktop/WindowsDesktopNetworkInformationImpl.cpp
    pal/desktop/WindowsDesktopSystemInformationImpl.cpp
    pal/desktop/WindowsEnvironmentInfo.hpp
    pal/desktop/NetworkDetector.cpp
    utils/StringConversion.cpp
  )

  # UTC module
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/utc")
    list(APPEND SRCS
      modules/utc/desktop/UtcHelpers.cpp
      modules/utc/UtcTelemetrySystem.cpp
    )
  endif()

else()
  message(FATAL_ERROR "No platform abstraction library configured. Set PAL_IMPLEMENTATION.")
endif()

# Filtering module
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/filter")
  list(APPEND SRCS
    modules/filter/CompliantByDefaultEventFilterModule.cpp
    modules/filter/CompliantByDefaultFilterApi.cpp
    modules/filter/LevelCheckingEventFilter.cpp
  )
endif()

create_source_files_groups_per_folder(${SRCS})

# Linux and Android require pthreads
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  find_package(Threads REQUIRED)
endif()

################################################################################################
# Build the library
################################################################################################
if(BUILD_SHARED_LIBS)
  message(STATUS "Building shared SDK library")
  add_library(mat SHARED ${SRCS})
else()
  message(STATUS "Building static SDK library")
  add_library(mat STATIC ${SRCS})
endif()

# Windows export/import decoration (MATSDK_LIBABI in lib/include/public/ctmacros.hpp).
# The SDK has no .def file, so __declspec(dllexport)/(dllimport) is the sole export
# mechanism, and it must follow the actual linkage:
#  * shared: export the public API from the SDK build (PRIVATE), and make
#    consumers import it. The INTERFACE define is carried by the installed
#    MSTelemetry::mat target, so find_package() + link gives consumers
#    dllimport automatically -- no consumer-side configuration.
#  * static: decorate nothing (define on both sides), so the SDK's public symbols
#    are NOT re-exported by a consumer DLL that absorbs this static lib.
if(WIN32)
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(mat PRIVATE MATSDK_SHARED_LIB=1 INTERFACE MATSDK_IMPORT_LIB=1)
  else()
    target_compile_definitions(mat PUBLIC MATSDK_STATIC_LIB=1)
  endif()
endif()

# Target-based include paths for vcpkg / install workflow.
# PUBLIC propagates to consumers; PRIVATE is SDK-internal only.
# BUILD_INTERFACE is used during the SDK build; INSTALL_INTERFACE is used
# by consumers after cmake --install.
target_include_directories(mat
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mat>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/pal
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

if(APPLE AND BUILD_OBJC_WRAPPER)
  if(MATSDK_OBJC_PRIVACYGUARD_AVAILABLE)
    target_compile_definitions(mat PRIVATE MATSDK_OBJC_PRIVACYGUARD_AVAILABLE=1)
  else()
    target_compile_definitions(mat PRIVATE MATSDK_OBJC_PRIVACYGUARD_AVAILABLE=0)
  endif()

  if(MATSDK_OBJC_SANITIZER_AVAILABLE)
    target_compile_definitions(mat PRIVATE MATSDK_OBJC_SANITIZER_AVAILABLE=1)
  else()
    target_compile_definitions(mat PRIVATE MATSDK_OBJC_SANITIZER_AVAILABLE=0)
  endif()
endif()

################################################################################################
# Link dependencies
################################################################################################
# TODO: allow adding "${Tcmalloc_LIBRARIES}" to target_link_libraries for memory leak debugging
# (USE_TCMALLOC / FindTcmalloc.cmake are configured for Debug builds in the root CMakeLists.txt,
# but the library is not yet linked here).
if(MATSDK_USE_VCPKG_DEPS)
  # vcpkg mode: all deps resolved via find_package() in root CMakeLists.txt
  # These are PUBLIC so static-library consumers get the transitive link set
  # through the exported MSTelemetry::mat target.
  target_link_libraries(mat
    PUBLIC
      unofficial::sqlite3::sqlite3
      ZLIB::ZLIB
      nlohmann_json::nlohmann_json
      ${LIBS}
  )
else()
  # Legacy mode: use vendored or system-installed deps
  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Android NDK has no system sqlite3 or zlib — build from bundled source.
    add_library(sqlite3_bundled STATIC "${CMAKE_CURRENT_SOURCE_DIR}/../sqlite/sqlite3.c")
    target_include_directories(sqlite3_bundled PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../sqlite")
    set_target_properties(sqlite3_bundled PROPERTIES POSITION_INDEPENDENT_CODE ON)
    # Guard bundled sqlite3 against toolchain or environment flags that imply finite-math-only (uses INFINITY macro).
    # Also suppress warnings treated as errors in vendored code.
    target_compile_options(sqlite3_bundled PRIVATE -fno-finite-math-only -Wno-unused-function)

    # Build zlib from bundled source.
    add_library(zlib_bundled STATIC
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/adler32.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/compress.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/crc32.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/deflate.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/gzclose.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/gzlib.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/gzread.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/gzwrite.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/infback.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/inffast.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/inflate.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/inftrees.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/trees.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/uncompr.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zutil.c"
    )
    target_include_directories(zlib_bundled PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../zlib")
    set_target_properties(zlib_bundled PROPERTIES POSITION_INDEPENDENT_CODE ON)
    # Bundled zlib compiles the pristine sources without zlib's configure step,
    # so tell it <unistd.h> is available (Android is POSIX). This gives gz*.c the
    # real POSIX declarations for read/write/lseek/close instead of relying on
    # implicit (int-returning) declarations.
    target_compile_definitions(zlib_bundled PRIVATE Z_HAVE_UNISTD_H)

    target_link_libraries(mat PRIVATE sqlite3_bundled zlib_bundled ${LIBS})
  elseif(PAL_IMPLEMENTATION STREQUAL "WIN32")
    # Windows legacy: vendored sqlite/zlib headers are included via
    # include_directories in the PAL section above; link only ${LIBS}
    # (e.g. CURL if needed — sqlite/zlib come from .vcxproj references).
    target_link_libraries(mat PRIVATE ${LIBS})
  else()
    # Linux/macOS legacy: link system-installed sqlite3 and zlib
    if(EXISTS "/usr/local/lib/libsqlite3.a")
      set(MATSDK_SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
    elseif(EXISTS "/usr/local/opt/sqlite/lib/libsqlite3.a")
      set(MATSDK_SQLITE3_LIB "/usr/local/opt/sqlite/lib/libsqlite3.a")
    elseif(EXISTS "/opt/homebrew/opt/sqlite/lib/libsqlite3.a")
      set(MATSDK_SQLITE3_LIB "/opt/homebrew/opt/sqlite/lib/libsqlite3.a")
    else()
      # find_package(SQLite3) needs CMake >= 3.14, guaranteed by the project floor;
      # SQLite::SQLite3 is an imported target carrying its own include dirs.
      find_package(SQLite3 REQUIRED)
      set(MATSDK_SQLITE3_LIB SQLite::SQLite3)
    endif()

    find_package(ZLIB REQUIRED)
    target_include_directories(mat PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(mat PRIVATE ${MATSDK_SQLITE3_LIB} ZLIB::ZLIB ${LIBS})
  endif()
endif()

# Platform-specific link dependencies
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  target_link_libraries(mat PUBLIC "${CMAKE_THREAD_LIBS_INIT}" "${CMAKE_DL_LIBS}")
  if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(mat PUBLIC "-pthread")
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_link_libraries(mat PUBLIC log)
  endif()
elseif(PAL_IMPLEMENTATION STREQUAL "WIN32")
  target_link_libraries(mat PUBLIC wininet crypt32 ws2_32)
elseif(APPLE)
  target_link_libraries(mat PUBLIC
    "-framework CoreFoundation"
    "-framework Foundation"
    "-framework CFNetwork"
    "-framework Network"
    "-framework SystemConfiguration"
  )
  if(BUILD_IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(mat PUBLIC "-framework UIKit")
  else()
    target_link_libraries(mat PUBLIC "-framework IOKit")
  endif()
endif()

################################################################################################
# Installation
################################################################################################
# The CMake package config / export workflow is used by vcpkg and any CMake-based
# consumer that does find_package(MSTelemetry).  Legacy (non-vcpkg) builds install
# via install.sh or MSBuild output directories and don't need this.
if(MATSDK_USE_VCPKG_DEPS)
  install(TARGETS mat
    EXPORT MSTelemetryTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  message(STATUS "Library will be installed to ${CMAKE_INSTALL_LIBDIR}")

  # Generate and install CMake package config files
  install(EXPORT MSTelemetryTargets
    FILE MSTelemetryTargets.cmake
    NAMESPACE MSTelemetry::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MSTelemetry
  )

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MSTelemetryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MSTelemetry
  )

  if(NOT DEFINED MATSDK_BUILD_VERSION OR MATSDK_BUILD_VERSION STREQUAL "")
    message(FATAL_ERROR "MATSDK_BUILD_VERSION is not set. Cannot generate package version file.")
  endif()

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfigVersion.cmake"
    VERSION ${MATSDK_BUILD_VERSION}
    COMPATIBILITY AnyNewerVersion
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MSTelemetry
  )
else()
  # Legacy install: just put the library and headers in standard locations
  install(TARGETS mat
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  message(STATUS "Library will be installed to ${CMAKE_INSTALL_LIBDIR}")
endif()

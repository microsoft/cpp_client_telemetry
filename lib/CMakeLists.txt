# Honor visibility properties for all target types
cmake_policy(SET CMP0063 NEW)

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
  offline/OfflineStorageHandler.cpp
  offline/LogSessionDataProvider.cpp
  backoff/IBackoff.cpp
  pal/PAL.cpp
  pal/TaskDispatcher_CAPI.cpp
  pal/WorkerThread.cpp
  decoder/PayloadDecoder.cpp
)

if(MATSDK_ANDROID_USE_ROOM)
  list(APPEND SRCS offline/OfflineStorage_Room.cpp)
else()
  list(APPEND SRCS offline/OfflineStorage_SQLite.cpp)
endif()

if(MATSDK_BUILD_JNI_WRAPPER)
  list(APPEND SRCS
    jni/JniConvertors.cpp
    jni/LogManager_jni.cpp
    jni/Logger_jni.cpp
    jni/SemanticContext_jni.cpp
    jni/Utils_jni.cpp)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/dataviewer/")
    list(APPEND SRCS jni/LogManagerDDVController_jni.cpp)
  endif()
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/"
      AND MATSDK_BUILD_PRIVACYGUARD)
    list(APPEND SRCS jni/PrivacyGuard_jni.cpp)
  endif()
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/signals/"
      AND MATSDK_BUILD_SIGNALS)
    list(APPEND SRCS jni/Signals_jni.cpp)
  endif()
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer/"
      AND MATSDK_BUILD_SANITIZER)
    list(APPEND SRCS jni/Sanitizer_jni.cpp)
  endif()
endif()

# Support for Azure Monitor / Application Insights
if(MATSDK_BUILD_AZMON)
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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/" AND MATSDK_BUILD_PRIVACYGUARD)
  list(APPEND SRCS
    modules/privacyguard/PrivacyGuard.cpp
    modules/privacyguard/RegisteredFileTypes.cpp
    modules/privacyguard/SummaryStatistics.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/liveeventinspector/" AND MATSDK_BUILD_LIVEEVENTINSPECTOR)
  list(APPEND SRCS
    modules/liveeventinspector/LiveEventInspector.cpp
    modules/liveeventinspector/LiveEventInspector.hpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/cds/" AND MATSDK_BUILD_CDS)
  target_compile_definitions(matsdk_internal_config INTERFACE HAVE_MAT_CDS)
  list(APPEND SRCS
    modules/cds/CdsFactory.hpp
    modules/cds/CdsFactory.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/signals/" AND MATSDK_BUILD_SIGNALS)
  list(APPEND SRCS
    modules/signals/Signals.cpp
    modules/signals/SignalsEncoder.cpp
  )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer/" AND MATSDK_BUILD_SANITIZER)
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
    if(MATSDK_PLATFORM_IOS)
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
    if(MATSDK_BUILD_APPLE_HTTP OR MATSDK_PLATFORM_IOS)
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
      if(MATSDK_ANDROID_USES_CURL)
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
  if(APPLE AND MATSDK_BUILD_OBJC_WRAPPER)
    message(STATUS "Include ObjC Wrappers")
    set(OBJC_WRAPPER_SRCS
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
        list(APPEND OBJC_WRAPPER_SRCS
        ../wrappers/obj-c/ODWDiagnosticDataViewer.mm
        )
    endif()
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard/" AND MATSDK_BUILD_PRIVACYGUARD)
          set(MATSDK_OBJC_PRIVACYGUARD_AVAILABLE ON)
          list(APPEND OBJC_WRAPPER_SRCS
          ../wrappers/obj-c/ODWPrivacyGuard.mm
            )
      endif()
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer/" AND MATSDK_BUILD_SANITIZER)
        set(MATSDK_OBJC_SANITIZER_AVAILABLE ON)
        list(APPEND OBJC_WRAPPER_SRCS
        ../wrappers/obj-c/ODWSanitizer.mm
        )
      endif()
    list(APPEND SRCS ${OBJC_WRAPPER_SRCS})
  endif()

  if(APPLE AND MATSDK_BUILD_SWIFT_WRAPPER)
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
target_compile_definitions(matsdk_internal_config INTERFACE
  _UNICODE
  UNICODE
  WIN32
  MATSDK_PLATFORM_WINDOWS=1
  _UTC_SDK
  USE_BOND
  _WINDOWS
  _USRDLL
  WINVER=_WIN32_WINNT_WIN7)
target_compile_options(matsdk_internal_config INTERFACE /U_MBCS)
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

if(APPLE)
  set(_matsdk_objcxx_sources ${SRCS})
  list(FILTER _matsdk_objcxx_sources INCLUDE REGEX "\\.mm$")
  if(_matsdk_objcxx_sources)
    set_source_files_properties(${_matsdk_objcxx_sources}
      PROPERTIES COMPILE_OPTIONS "-fobjc-arc;-Wno-error=shorten-64-to-32")
  endif()
endif()

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
target_link_libraries(mat PRIVATE
  $<BUILD_INTERFACE:matsdk_build_options>
  $<BUILD_INTERFACE:matsdk_internal_config>)
set_target_properties(mat PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(BUILD_SHARED_LIBS)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(mat PRIVATE
      $<$<NOT:$<CONFIG:Debug>>:-s>
      $<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>)
  elseif(APPLE)
    target_link_options(mat PRIVATE
      $<$<NOT:$<CONFIG:Debug>>:-Wl,-dead_strip>)
  endif()
endif()
if(APPLE)
  target_compile_options(mat PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang>:-Wno-error=shorten-64-to-32>)
endif()
if(NOT TARGET MSTelemetry::mat)
  add_library(MSTelemetry::mat ALIAS mat)
endif()

# Public-API export decoration (MATSDK_LIBABI in lib/include/public/ctmacros.hpp).
# The SDK has no .def file, so __declspec(dllexport)/(dllimport) on Windows and
# __attribute__((visibility("default"))) elsewhere are the sole export mechanisms,
# and the decoration must follow the actual linkage:
#  * shared: the SDK's own translation units export the public API
#    (MATSDK_SHARED_LIB, PRIVATE). On Windows, consumers must additionally import
#    it -- the INTERFACE MATSDK_IMPORT_LIB is carried by the installed
#    MSTelemetry::mat target, so find_package() + link gives consumers dllimport
#    automatically with no consumer-side configuration. Non-Windows consumers need
#    nothing: they call symbols exported by libmat.so/.dylib.
#  * static: decorate nothing, so the SDK's public symbols are NOT re-exported by
#    a consumer DLL/.so that statically absorbs this library. Windows needs an
#    explicit (empty) MATSDK_STATIC_LIB; elsewhere the empty MATSDK_LIBABI default
#    plus -fvisibility=hidden (root CMakeLists.txt) already hides them.
if(BUILD_SHARED_LIBS)
  target_compile_definitions(mat PRIVATE MATSDK_SHARED_LIB=1)
  if(WIN32)
    target_compile_definitions(mat INTERFACE MATSDK_IMPORT_LIB=1)
  endif()
elseif(WIN32)
  target_compile_definitions(mat PUBLIC MATSDK_STATIC_LIB=1)
endif()

# Target-based include paths for vcpkg / install workflow.
# PUBLIC propagates to consumers; PRIVATE is SDK-internal only.
# BUILD_INTERFACE is used during the SDK build; INSTALL_INTERFACE is used
# by consumers after cmake --install.
#
# The public headers are added in a separate SYSTEM call: SYSTEM marks them as
# system includes for consumers, so a consumer building with -Wall -Wextra
# -Werror is not broken by warnings originating inside the SDK's headers (e.g.
# -Wpedantic variadic-macro or -Wconversion diagnostics). find_package consumers
# already treat an imported target's includes as system; SYSTEM extends the same
# courtesy to add_subdirectory/FetchContent consumers. The PRIVATE internal
# include dirs are deliberately kept out of this SYSTEM call so the SDK's own
# -Werror build still diagnoses warnings in its internal headers.
target_include_directories(mat
  SYSTEM PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mat>
)
target_include_directories(mat
  PRIVATE
    ${PROJECT_SOURCE_DIR}/bondlite/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mat
    ${CMAKE_CURRENT_SOURCE_DIR}/pal
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/exp
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/dataviewer
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/privacyguard
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/liveeventinspector
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/cds
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/signals
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/sanitizer
)
if(NOT MATSDK_USES_NLOHMANN_TARGET)
  target_include_directories(mat PRIVATE ${PROJECT_SOURCE_DIR})
endif()

if(APPLE AND MATSDK_BUILD_OBJC_WRAPPER)
  if(BUILD_SHARED_LIBS AND OBJC_WRAPPER_SRCS)
    # The root CMakeLists.txt applies -fvisibility=hidden globally to shrink the
    # exported symbol table of the core C++ SDK. For Objective-C that also hides
    # the wrapper class symbols (_OBJC_CLASS_$_ODW*), which are public API on
    # Apple: a shared libmat.dylib would export no ODW* classes and consumers
    # would fail to link (undefined _OBJC_CLASS_$_...). Re-export just the wrapper
    # translation units with default visibility; the C++ core stays hidden.
    set_source_files_properties(${OBJC_WRAPPER_SRCS}
      PROPERTIES COMPILE_FLAGS "-fvisibility=default")
  endif()

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
# --- Minimal SQLite -----------------------------------------------------------
# The SDK uses SQLite only for its offline event-storage cache: plain tables,
# indexes, transactions, WAL, autovacuum/VACUUM, a handful of PRAGMAs, and one
# custom UTF-8 SQL function. None of SQLite's optional subsystems are needed, so
# when MATSDK_SQLITE_PROVIDER=MINIMAL the bundled SQLite is compiled with these
# options to strip out everything the SDK does not use (~10% smaller SQLite code).
# They are all amalgamation-safe (no grammar/parser regeneration) and validated
# against the offline-storage unit tests.
#
# Two options are deliberately NOT stripped because the SDK depends on them:
#  * SQLITE_OMIT_AUTOINIT: the skipSqliteInitAndShutdown runtime config lets the
#    host own SQLite's lifecycle and skip the SDK's explicit sqlite3_initialize()
#    (lib/offline/SQLiteWrapper.hpp). With a private bundled SQLite the host cannot
#    initialize the SDK's copy, so auto-init must remain on.
#  * SQLITE_DEFAULT_MEMSTATUS=0: the SDK arms a soft heap limit via
#    sqlite3_soft_heap_limit64(cacheMemorySizeLimitInBytes) on every open, and that
#    limit is only enforced while memory statistics are enabled. Disabling them
#    would silently turn the configured memory cap into a no-op.
set(MATSDK_SQLITE_MINIMAL_DEFS
  SQLITE_DQS=0
  SQLITE_THREADSAFE=1
  SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
  SQLITE_DEFAULT_FOREIGN_KEYS=0
  SQLITE_LIKE_DOESNT_MATCH_BLOBS
  SQLITE_MAX_EXPR_DEPTH=0
  SQLITE_MAX_MMAP_SIZE=0
  SQLITE_USE_ALLOCA
  SQLITE_OMIT_DEPRECATED
  SQLITE_OMIT_PROGRESS_CALLBACK
  SQLITE_OMIT_SHARED_CACHE
  SQLITE_OMIT_LOAD_EXTENSION
  SQLITE_OMIT_DECLTYPE
  SQLITE_OMIT_JSON
  SQLITE_OMIT_TRACE
  SQLITE_OMIT_COMPLETE
  SQLITE_OMIT_GET_TABLE
  SQLITE_OMIT_TCL_VARIABLE
  SQLITE_OMIT_EXPLAIN
  SQLITE_OMIT_AUTHORIZATION
  SQLITE_OMIT_DESERIALIZE
  SQLITE_OMIT_INTROSPECTION_PRAGMAS
  SQLITE_UNTESTABLE
)

if(MATSDK_BUNDLE_SQLITE AND NOT TARGET sqlite3_bundled)
  add_library(sqlite3_bundled STATIC "${CMAKE_CURRENT_SOURCE_DIR}/../sqlite/sqlite3.c")
  # Consumers of MSTelemetry::mat never include sqlite3.h (it is an internal
  # implementation detail), so the header path is only needed while building the
  # SDK itself -- wrap it in BUILD_INTERFACE so install(EXPORT) stays valid.
  target_include_directories(sqlite3_bundled PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../sqlite>")
  set_target_properties(sqlite3_bundled PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden)
  if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "MINIMAL")
    # Feature-stripped build: apply the minimal definitions.
    target_compile_definitions(sqlite3_bundled PRIVATE ${MATSDK_SQLITE_MINIMAL_DEFS})
  endif()
  if(USE_ONEDS_SECURE_MEM_FUNCTIONS)
    target_compile_definitions(sqlite3_bundled PRIVATE
      USE_ONEDS_SECURE_MEM_FUNCTIONS)
  endif()
  if(APPLE AND MATSDK_PLATFORM_IOS)
    # SQLite already resolves this to false on Apple mobile platforms. Define it
    # explicitly so the amalgamation does not emit its gethostuuid warning under
    # consumer-provided warnings-as-errors.
    target_compile_definitions(sqlite3_bundled PRIVATE HAVE_GETHOSTUUID=0)
  endif()
  if(MSVC)
    # Silence the vendored amalgamation's warnings (/w) and turn off
    # warning-as-error (/WX-) for this third-party translation unit, so the SDK's
    # /WX does not promote any amalgamation warning that survives /w to an error.
    target_compile_options(sqlite3_bundled PRIVATE
      /w /WX- /Gy
      $<$<C_COMPILER_ID:MSVC>:/Gw>)
  elseif(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "MINIMAL")
    # -w disables all warnings for this vendored translation unit so the SDK's
    # -Werror does not fire on amalgamation code (the OMIT_* options leave some
    # debug-build macros expanding to empty/unused statements). -fno-finite-math-only:
    # the amalgamation relies on the INFINITY macro, which -ffast-math /
    # -ffinite-math-only would break.
    target_compile_options(sqlite3_bundled PRIVATE
      -w -fno-finite-math-only -ffunction-sections
      $<$<NOT:$<C_COMPILER_ID:AppleClang>>:-fdata-sections>)
  else()
    # Unstripped vendored build (Android legacy): keep the existing narrower
    # warning suppression. -fno-finite-math-only guards the INFINITY macro.
    target_compile_options(sqlite3_bundled PRIVATE
      -fno-finite-math-only -Wno-unused-function -ffunction-sections
      $<$<NOT:$<C_COMPILER_ID:AppleClang>>:-fdata-sections>)
  endif()
endif()
if(MATSDK_BUNDLE_SQLITE AND NOT TARGET SQLite::SQLite3)
  add_library(SQLite::SQLite3 ALIAS sqlite3_bundled)
endif()

if(MATSDK_BUNDLE_ZLIB AND NOT TARGET zlib_bundled)
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
  target_include_directories(zlib_bundled PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../zlib>")
  set_target_properties(zlib_bundled PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden)
  if(NOT WIN32)
    target_compile_definitions(zlib_bundled PRIVATE Z_HAVE_UNISTD_H)
  else()
    target_compile_definitions(zlib_bundled PRIVATE ZLIB_WINAPI)
    target_compile_definitions(zlib_bundled INTERFACE
      $<BUILD_INTERFACE:ZLIB_WINAPI>)
  endif()
  if(MSVC)
    target_compile_options(zlib_bundled PRIVATE
      /w /WX- /Gy
      $<$<C_COMPILER_ID:MSVC>:/Gw>)
  else()
    target_compile_options(zlib_bundled PRIVATE
      -w -ffunction-sections
      $<$<NOT:$<C_COMPILER_ID:AppleClang>>:-fdata-sections>)
  endif()
endif()
if(MATSDK_BUNDLE_ZLIB AND NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlib_bundled)
endif()

if(NOT MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "NONE"
   AND NOT TARGET SQLite::SQLite3)
  message(FATAL_ERROR
    "SQLite::SQLite3 was not resolved for provider ${MATSDK_SQLITE_PROVIDER_RESOLVED}.")
endif()
if(NOT TARGET ZLIB::ZLIB)
  message(FATAL_ERROR
    "ZLIB::ZLIB was not resolved for provider ${MATSDK_ZLIB_PROVIDER_RESOLVED}.")
endif()

if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "NONE")
  # Room provides Android offline storage; no SQLite dependency is needed.
elseif(MATSDK_BUNDLE_SQLITE)
  target_link_libraries(mat PRIVATE sqlite3_bundled)
else()
  matsdk_add_interface_dependency(
    matsdk_sqlite_dependency SQLite::SQLite3)
  target_link_libraries(mat PRIVATE
    "$<BUILD_INTERFACE:matsdk_sqlite_dependency>"
    "$<INSTALL_INTERFACE:MSTelemetry::sqlite_dependency>")
endif()
if(MATSDK_BUNDLE_ZLIB)
  target_link_libraries(mat PRIVATE zlib_bundled)
else()
  matsdk_add_interface_dependency(
    matsdk_zlib_dependency ZLIB::ZLIB)
  target_link_libraries(mat PRIVATE
    "$<BUILD_INTERFACE:matsdk_zlib_dependency>"
    "$<INSTALL_INTERFACE:MSTelemetry::zlib_dependency>")
endif()
if(MATSDK_CURL_LINK_TARGET)
  matsdk_add_interface_dependency(
    matsdk_curl_dependency ${MATSDK_CURL_LINK_TARGET})
  target_link_libraries(mat PRIVATE
    "$<BUILD_INTERFACE:matsdk_curl_dependency>"
    "$<INSTALL_INTERFACE:MSTelemetry::curl_dependency>")
endif()
if(MATSDK_USES_NLOHMANN_TARGET)
  matsdk_add_interface_dependency(
    matsdk_nlohmann_dependency nlohmann_json::nlohmann_json)
  target_link_libraries(mat PRIVATE
    "$<BUILD_INTERFACE:matsdk_nlohmann_dependency>")
endif()

# TODO: allow adding "${Tcmalloc_LIBRARIES}" to target_link_libraries for memory leak debugging
# (USE_TCMALLOC / FindTcmalloc.cmake are configured for Debug builds in the root CMakeLists.txt,
# but the library is not yet linked here).

# Platform-specific link dependencies
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  target_link_libraries(mat PUBLIC Threads::Threads "${CMAKE_DL_LIBS}")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    target_link_libraries(mat PUBLIC atomic)
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
  if(MATSDK_PLATFORM_IOS)
    target_link_libraries(mat PUBLIC "-framework UIKit")
  else()
    target_link_libraries(mat PUBLIC "-framework IOKit")
  endif()
endif()

################################################################################################
# Installation
################################################################################################
# A static libmat propagates its private static dependencies at the final link,
# so install bundled archives beside it in both source and vcpkg workflows.
set(MATSDK_EXPORT_TARGETS mat)
set(MATSDK_AUX_INSTALL_TARGETS)
get_target_property(_mat_type mat TYPE)
if(_mat_type STREQUAL "STATIC_LIBRARY")
  if(MATSDK_BUNDLE_SQLITE AND TARGET sqlite3_bundled)
    list(APPEND MATSDK_EXPORT_TARGETS sqlite3_bundled)
  endif()
  if(MATSDK_BUNDLE_ZLIB AND TARGET zlib_bundled)
    list(APPEND MATSDK_EXPORT_TARGETS zlib_bundled)
  endif()
  foreach(_matsdk_fetched_target IN LISTS MATSDK_FETCHED_CURL_TARGETS)
    if(TARGET ${_matsdk_fetched_target})
      list(APPEND MATSDK_AUX_INSTALL_TARGETS ${_matsdk_fetched_target})
    endif()
  endforeach()
endif()

set(MATSDK_CONFIG_STATIC_PACKAGE FALSE)
if(_mat_type STREQUAL "STATIC_LIBRARY")
  set(MATSDK_CONFIG_STATIC_PACKAGE TRUE)
endif()

install(TARGETS ${MATSDK_EXPORT_TARGETS}
  EXPORT MSTelemetryTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(MATSDK_AUX_INSTALL_TARGETS)
  install(TARGETS ${MATSDK_AUX_INSTALL_TARGETS}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

message(STATUS "Library will be installed to ${CMAKE_INSTALL_LIBDIR}")

# Generate and install CMake package config files for every CMake build.
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

set(_matsdk_package_config_files
  "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/MSTelemetryConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MatsdkDependencyTargets.cmake")
if(APPLE)
  list(APPEND _matsdk_package_config_files
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MatsdkAppleSystemDeps.cmake")
endif()
install(FILES ${_matsdk_package_config_files}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MSTelemetry
)

if(CMAKE_SYSTEM_NAME STREQUAL "Android" AND MATSDK_ANDROID_USES_JAVA_HTTP)
  install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/android_build/maesdk/src/main/java/com/microsoft/applications/events/HttpClient.java"
    "${CMAKE_CURRENT_SOURCE_DIR}/android_build/maesdk/src/main/java/com/microsoft/applications/events/HttpClientRequest.java"
    DESTINATION
      "${CMAKE_INSTALL_DATADIR}/cpp-client-telemetry/android/java/com/microsoft/applications/events"
  )
endif()

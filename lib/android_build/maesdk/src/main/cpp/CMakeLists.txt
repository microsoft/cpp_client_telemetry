# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 11)

# Enable Azure Monitor / Application Insights end-point support
option(BUILD_AZMON        "Build for Azure Monitor" YES)
option(BUILD_PRIVACYGUARD "Build Privacy Guard"     YES)

string(REPLACE "/lib/android_build/maesdk/src/main/cpp" "" SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if (USE_CURL)
        add_definitions(-DHAVE_MAT_CURL_HTTP_CLIENT)
        find_package(CURL REQUIRED)
endif()

set(TARGETNAME maesdk)

set(SRCS
        ${SDK_ROOT}/lib/api/AllowedLevelsCollection.cpp
        ${SDK_ROOT}/lib/api/AuthTokensController.cpp
        ${SDK_ROOT}/lib/api/ContextFieldsProvider.cpp
        ${SDK_ROOT}/lib/api/CorrelationVector.cpp
        ${SDK_ROOT}/lib/api/DataViewerCollection.cpp
        ${SDK_ROOT}/lib/api/ILogConfiguration.cpp
        ${SDK_ROOT}/lib/api/LogConfiguration.cpp
        ${SDK_ROOT}/lib/api/LogManager.cpp
        ${SDK_ROOT}/lib/api/LogManagerFactory.cpp
        ${SDK_ROOT}/lib/api/LogManagerImpl.cpp
        ${SDK_ROOT}/lib/api/LogManagerProvider.cpp
        ${SDK_ROOT}/lib/api/LogSessionData.cpp
        ${SDK_ROOT}/lib/api/Logger.cpp
        ${SDK_ROOT}/lib/api/capi.cpp
        ${SDK_ROOT}/lib/backoff/IBackoff.cpp
        ${SDK_ROOT}/lib/bond/BondSerializer.cpp
        ${SDK_ROOT}/lib/callbacks/DebugSource.cpp
        ${SDK_ROOT}/lib/compression/HttpDeflateCompression.cpp
        ${SDK_ROOT}/lib/decorators/BaseDecorator.cpp
        ${SDK_ROOT}/lib/filter/EventFilterCollection.cpp
        ${SDK_ROOT}/lib/http/HttpClientFactory.cpp
        ${SDK_ROOT}/lib/http/HttpClientManager.cpp
        ${SDK_ROOT}/lib/http/HttpRequestEncoder.cpp
        ${SDK_ROOT}/lib/http/HttpResponseDecoder.cpp
        ${SDK_ROOT}/lib/jni/JniConvertors.cpp
        ${SDK_ROOT}/lib/jni/LogManager_jni.cpp
        ${SDK_ROOT}/lib/jni/Logger_jni.cpp
        ${SDK_ROOT}/lib/jni/SemanticContext_jni.cpp
        ${SDK_ROOT}/lib/jni/Utils_jni.cpp
        ${SDK_ROOT}/lib/offline/MemoryStorage.cpp
        ${SDK_ROOT}/lib/offline/LogSessionDataProvider.cpp
        ${SDK_ROOT}/lib/offline/OfflineStorageFactory.cpp
        ${SDK_ROOT}/lib/offline/OfflineStorageHandler.cpp
        ${SDK_ROOT}/lib/offline/StorageObserver.cpp
        ${SDK_ROOT}/lib/packager/BondSplicer.cpp
        ${SDK_ROOT}/lib/packager/Packager.cpp
        ${SDK_ROOT}/lib/pal/InformationProviderImpl.cpp
        ${SDK_ROOT}/lib/pal/PAL.cpp
        ${SDK_ROOT}/lib/pal/TaskDispatcher_CAPI.cpp
        ${SDK_ROOT}/lib/pal/WorkerThread.cpp
        ${SDK_ROOT}/lib/pal/posix/DeviceInformationImpl_Android.cpp
        ${SDK_ROOT}/lib/pal/posix/NetworkInformationImpl_Android.cpp
        ${SDK_ROOT}/lib/pal/posix/SystemInformationImpl_Android.cpp
        ${SDK_ROOT}/lib/pal/posix/sysinfo_sources.cpp
        ${SDK_ROOT}/lib/stats/MetaStats.cpp
        ${SDK_ROOT}/lib/stats/Statistics.cpp
        ${SDK_ROOT}/lib/system/EventProperties.cpp
        ${SDK_ROOT}/lib/system/EventProperty.cpp
        ${SDK_ROOT}/lib/system/TelemetrySystem.cpp
        ${SDK_ROOT}/lib/tpm/DeviceStateHandler.cpp
        ${SDK_ROOT}/lib/tpm/TransmissionPolicyManager.cpp
        ${SDK_ROOT}/lib/tpm/TransmitProfiles.cpp
        ${SDK_ROOT}/lib/utils/FileUtils.cpp
        ${SDK_ROOT}/lib/utils/StringUtils.cpp
        ${SDK_ROOT}/lib/utils/ZlibUtils.cpp
        ${SDK_ROOT}/lib/utils/Utils.cpp
)

# Support for Azure Monitor / Application Insights
if (BUILD_AZMON)
    include(${SDK_ROOT}/lib/modules/azmon/CMakeLists.txt OPTIONAL)
endif()

if(EXISTS ${SDK_ROOT}/lib/modules/dataviewer/)
        list(APPEND SRCS
                ${SDK_ROOT}/lib/jni/LogManagerDDVController_jni.cpp
                ${SDK_ROOT}/lib/modules/dataviewer/DefaultDataViewer.cpp
                ${SDK_ROOT}/lib/modules/dataviewer/OnDisableNotificationCollection.cpp
        )
endif()

if(EXISTS ${SDK_ROOT}/lib/modules/privacyguard/ AND BUILD_PRIVACYGUARD)
        list(APPEND SRCS
                ${SDK_ROOT}/lib/jni/PrivacyGuard_jni.cpp
                ${SDK_ROOT}/lib/modules/privacyguard/SummaryStatistics.cpp
                ${SDK_ROOT}/lib/modules/privacyguard/PrivacyGuard.cpp
                ${SDK_ROOT}/lib/modules/privacyguard/RegisteredFileTypes.cpp
        )
endif()

if (USE_ROOM)
        add_definitions("-DUSE_ROOM")
        list(APPEND SRCS ${SDK_ROOT}/lib/offline/OfflineStorage_Room.cpp)
else()
        list(APPEND SRCS
                ${SDK_ROOT}/lib/offline/OfflineStorage_SQLite.cpp
                ${SDK_ROOT}/sqlite/sqlite3.c
                )
endif()

if (USE_CURL)
        list(APPEND SRCS ${SDK_ROOT}/lib/http/HttpClient_Curl.cpp)
else()
        list(APPEND SRCS ${SDK_ROOT}/lib/http/HttpClient_Android.cpp)
endif()

add_library(${TARGETNAME} ${SRCS})

target_include_directories(${TARGETNAME} PUBLIC
        ${SDK_ROOT}/lib
        ${SDK_ROOT}/lib/include/public
        ${SDK_ROOT}/lib/include
        ${SDK_ROOT}/lib/include/mat
        ${SDK_ROOT}/sqlite
        ${SDK_ROOT}lib/pal
        ${CURL_INCLUDE_DIRS})


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library(
        zlib
        z
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(maesdk PUBLIC
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${zlib}
        ${CURL_LIBRARIES}
        )

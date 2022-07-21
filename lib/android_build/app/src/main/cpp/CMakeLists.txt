# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 11)

string(REPLACE "/lib/android_build/app/src/main/cpp" "" SDK_ROOT ${CMAKE_SOURCE_DIR})

# TODO: migrate this from ${SDK_ROOT}/googletest to ${SDK_ROOT}/third_party/googletest
set (gmock_dir ${SDK_ROOT}/third_party/googletest/googlemock)
set (gtest_dir ${SDK_ROOT}/third_party/googletest/googletest)

include_directories(AFTER
        ${SDK_ROOT}/lib
        ${SDK_ROOT}/lib/include/public
        ${SDK_ROOT}/lib/include
        ${SDK_ROOT}/lib/include/mat
        ${SDK_ROOT}/sqlite
        ${SDK_ROOT}lib/pal
        "${gmock_dir}/include"
        "${gmock_dir}"
        "${gtest_dir}/include"
        # This directory is needed to build directly from Google
        # Test sources.
        "${gtest_dir}")

set(TESTS_COMMON_SRCS
        ${SDK_ROOT}/tests/common/Common.cpp
        ${SDK_ROOT}/tests/common/Mocks.cpp
        ${SDK_ROOT}/tests/common/Reactor.cpp
        ${SDK_ROOT}/lib/decoder/PayloadDecoder.cpp
        )

set(TESTS_SRCS
        ${SDK_ROOT}/tests/unittests/BackoffTests_ExponentialWithJitter.cpp
        ${SDK_ROOT}/tests/unittests/BondSplicerTests.cpp
        ${SDK_ROOT}/tests/unittests/ClockSkewManagerTests.cpp
        ${SDK_ROOT}/tests/unittests/ContextFieldsProviderTests.cpp
        ${SDK_ROOT}/tests/unittests/ControlPlaneProviderTests.cpp
        ${SDK_ROOT}/tests/unittests/CorrelationVectorTests.cpp
        ${SDK_ROOT}/tests/unittests/DataViewerCollectionTests.cpp
        ${SDK_ROOT}/tests/unittests/DebugEventSourceTests.cpp
        ${SDK_ROOT}/tests/unittests/DeviceStateHandlerTests.cpp
        ${SDK_ROOT}/tests/unittests/DiskLocalStorageTests.cpp
        ${SDK_ROOT}/tests/unittests/EventFilterCollectionTests.cpp
        ${SDK_ROOT}/tests/unittests/EventPropertiesStorageTests.cpp
        ${SDK_ROOT}/tests/unittests/EventPropertiesTests.cpp
        ${SDK_ROOT}/tests/unittests/GuidTests.cpp
        ${SDK_ROOT}/tests/unittests/HttpClientManagerTests.cpp
        ${SDK_ROOT}/tests/unittests/HttpClientTests.cpp
        ${SDK_ROOT}/tests/unittests/HttpDeflateCompressionTests.cpp
        ${SDK_ROOT}/tests/unittests/HttpRequestEncoderTests.cpp
        ${SDK_ROOT}/tests/unittests/HttpResponseDecoderTests.cpp
        ${SDK_ROOT}/tests/unittests/LoggerTests.cpp
        ${SDK_ROOT}/tests/unittests/LogManagerImplTests.cpp
        ${SDK_ROOT}/tests/unittests/LogSessionDataTests.cpp
        ${SDK_ROOT}/tests/unittests/Main.cpp
        ${SDK_ROOT}/tests/unittests/MemoryStorageTests.cpp
        ${SDK_ROOT}/tests/unittests/MetaStatsTests.cpp
        ${SDK_ROOT}/tests/unittests/OacrTests.cpp
        ${SDK_ROOT}/tests/unittests/OfflineStorageTests.cpp
        ${SDK_ROOT}/tests/unittests/OfflineStorageTests_Room.cpp
        ${SDK_ROOT}/tests/unittests/PackagerTests.cpp
        ${SDK_ROOT}/tests/unittests/PalTests.cpp
        ${SDK_ROOT}/tests/unittests/RouteTests.cpp
        ${SDK_ROOT}/tests/unittests/StringUtilsTests.cpp
        ${SDK_ROOT}/tests/unittests/TaskDispatcherCAPITests.cpp
        ${SDK_ROOT}/tests/unittests/TransmissionPolicyManagerTests.cpp
        ${SDK_ROOT}/tests/unittests/TransmitProfilesTests.cpp
        ${SDK_ROOT}/tests/unittests/UtilsTests.cpp
        )

find_package( ZLIB REQUIRED )
include_directories(
        ${ZLIB_INCLUDE_DIRS}
        ${SDK_ROOT}/tests
)

find_library(zlib-path z)

add_library(maesdk SHARED IMPORTED)
set_target_properties(maesdk PROPERTIES
        IMPORTED_LOCATION "${SDK_ROOT}/lib/android_build/maesdk/build/intermediates/cmake/debug/obj/${CMAKE_ANDROID_ARCH_ABI}/libmaesdk.so"
        IMPORTED_LOCATION_Release "${SDK_ROOT}/lib/android_build/maesdk/build/intermediates/cmake/debug/obj/${CMAKE_ANDROID_ARCH_ABI}/libmaesdk.so")

# include the other flavor of database: if maesdk builds with Room include sqlite
# if maesdk builds with native sqlite, include Room

if (USE_ROOM)
        set(OTHER_OFFLINE_SRCS
                ${SDK_ROOT}/lib/offline/OfflineStorage_SQLite.cpp
                ${SDK_ROOT}/sqlite/sqlite3.c
        )
else()
        set(OTHER_OFFLINE_SRCS
                ${SDK_ROOT}/lib/offline/OfflineStorage_Room.cpp
        )
endif()

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        ${OTHER_OFFLINE_SRCS}
        ${gtest_dir}/src/gtest-all.cc
        ${gmock_dir}/src/gmock-all.cc
        ${TESTS_COMMON_SRCS}
        ${TESTS_SRCS}
        )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        maesdk
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${zlib-path}
        )
        

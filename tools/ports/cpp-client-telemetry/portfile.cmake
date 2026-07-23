# In-repo overlay-port use should build the working tree under review instead of
# a pinned release -- this is what lets local port installs and tests exercise
# the SDK source + manifest together. The registry copy of this port is not under
# the SDK checkout, so it falls back to the pinned release below.
if(DEFINED ENV{MATSDK_VCPKG_SOURCE_DIR})
    set(SOURCE_PATH "$ENV{MATSDK_VCPKG_SOURCE_DIR}")
    if(NOT EXISTS "${SOURCE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR
            "MATSDK_VCPKG_SOURCE_DIR is set to '${SOURCE_PATH}', but no CMakeLists.txt "
            "was found there. It must point to a cpp_client_telemetry source checkout.")
    endif()
    message(STATUS "cpp-client-telemetry: building local source $ENV{MATSDK_VCPKG_SOURCE_DIR} (MATSDK_VCPKG_SOURCE_DIR is set)")
else()
    get_filename_component(_matsdk_overlay_source "${CURRENT_PORT_DIR}/../../.." ABSOLUTE)
endif()

if(NOT DEFINED SOURCE_PATH
   AND EXISTS "${_matsdk_overlay_source}/CMakeLists.txt"
   AND EXISTS "${_matsdk_overlay_source}/lib/CMakeLists.txt"
   AND EXISTS "${_matsdk_overlay_source}/tools/ports/cpp-client-telemetry/portfile.cmake")
    set(SOURCE_PATH "${_matsdk_overlay_source}")
    message(STATUS "cpp-client-telemetry: building in-repo overlay source ${SOURCE_PATH}")
endif()

if(NOT DEFINED SOURCE_PATH)
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO microsoft/cpp_client_telemetry
        REF v3.10.173.1
        SHA512 e55bc35274236f57757660073c4dccccab3462342c8566212f1df4bf8824295a2bb3d3d79a11f3950e7c9252641827e9dd3d7c28c421dea3bdaee277e4f2ce32
        HEAD_REF main
    )
endif()

# Determine if Apple HTTP should be used (no curl needed).
# Note: BUILD_APPLE_HTTP must remain ON for macOS/iOS because the vcpkg.json
# curl dependency is excluded on these platforms.
set(MATSDK_BUILD_APPLE_HTTP OFF)
if(VCPKG_TARGET_IS_OSX OR VCPKG_TARGET_IS_IOS)
  set(MATSDK_BUILD_APPLE_HTTP ON)
endif()

# iOS build options
set(MATSDK_BUILD_IOS OFF)
if(VCPKG_TARGET_IS_IOS)
  set(MATSDK_BUILD_IOS ON)
endif()

set(MATSDK_ANDROID_HTTP_CLIENT AUTO)
if(VCPKG_TARGET_IS_ANDROID)
  file(READ "${SOURCE_PATH}/CMakeLists.txt" _matsdk_root_cmake)
  if(NOT _matsdk_root_cmake MATCHES "MATSDK_ANDROID_HTTP_CLIENT")
    message(FATAL_ERROR
      "Android vcpkg builds require a cpp-client-telemetry source revision that "
      "supports MATSDK_ANDROID_HTTP_CLIENT. Update this port's REF/SHA512 to a "
      "newer SDK release, or set MATSDK_VCPKG_SOURCE_DIR to a local checkout "
      "that contains the Android Java transport selector.")
  endif()
  if("android-curl-openssl" IN_LIST FEATURES OR "android-curl-mbedtls" IN_LIST FEATURES)
    set(MATSDK_ANDROID_HTTP_CLIENT CURL)
  endif()
endif()

# curl-openssl/curl-mbedtls choose the Linux TLS backend. Android defaults to
# Java/JNI HTTP and uses separate explicit android-curl-* features for its curl
# escape hatch. vcpkg cannot express mutual exclusivity or "exactly one of", so
# validate it here -- but only where curl is actually used, to avoid failing
# legitimate cross-platform manifests on Windows/Apple.
set(_matsdk_http_features "")
if(VCPKG_TARGET_IS_ANDROID)
  set(_matsdk_http_feature_candidates android-curl-openssl android-curl-mbedtls)
else()
  set(_matsdk_http_feature_candidates curl-openssl curl-mbedtls)
endif()
foreach(_matsdk_http_feature ${_matsdk_http_feature_candidates})
  if(_matsdk_http_feature IN_LIST FEATURES)
    list(APPEND _matsdk_http_features ${_matsdk_http_feature})
  endif()
endforeach()
list(LENGTH _matsdk_http_features _matsdk_http_feature_count)
if(VCPKG_TARGET_IS_LINUX OR MATSDK_ANDROID_HTTP_CLIENT STREQUAL "CURL")
  if(_matsdk_http_feature_count GREATER 1)
    message(FATAL_ERROR
      "The curl HTTP backend features are mutually exclusive but multiple were "
      "selected. On Linux, use exactly one of curl-openssl/curl-mbedtls. On "
      "Android, use exactly one of android-curl-openssl/android-curl-mbedtls.")
  elseif(_matsdk_http_feature_count EQUAL 0 AND VCPKG_TARGET_IS_LINUX)
    # The built-in curl HTTP client requires exactly one TLS backend. The [core,...]
    # form drops the default curl-openssl, so fail fast (with a complete example)
    # rather than letting the SDK CMake fail later on a missing libcurl.
    message(FATAL_ERROR
      "On Linux the built-in curl HTTP client requires exactly one TLS backend "
      "feature, but none was selected. The [core,...] form drops the default "
      "curl-openssl feature, so re-add a curl backend together with a SQLite "
      "backend, e.g. "
      "cpp-client-telemetry[core,curl-mbedtls,system-sqlite] "
      "(or minimal-sqlite in place of system-sqlite).")
  elseif(_matsdk_http_feature_count EQUAL 0)
    message(FATAL_ERROR
      "On Android, MATSDK_ANDROID_HTTP_CLIENT=CURL requires exactly one explicit "
      "Android curl backend feature. Use android-curl-openssl or "
      "android-curl-mbedtls together with a SQLite backend, e.g. "
      "cpp-client-telemetry[core,android-curl-openssl,system-sqlite].")
  endif()
endif()

# minimal-sqlite -> -DMATSDK_MINIMAL_SQLITE=ON (private feature-stripped SQLite).
vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        minimal-sqlite MATSDK_MINIMAL_SQLITE
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DMATSDK_USE_VCPKG_DEPS=ON
        -DMATSDK_ANDROID_HTTP_CLIENT=${MATSDK_ANDROID_HTTP_CLIENT}
        -DBUILD_HEADERS=ON
        -DBUILD_LIBRARY=ON
        -DBUILD_TEST_TOOL=OFF
        -DBUILD_UNIT_TESTS=OFF
        -DBUILD_FUNC_TESTS=OFF
        -DBUILD_JNI_WRAPPER=OFF
        -DBUILD_OBJC_WRAPPER=OFF
        -DBUILD_SWIFT_WRAPPER=OFF
        -DBUILD_PACKAGE=OFF
        -DBUILD_VERSION=${VERSION}
        -DBUILD_APPLE_HTTP=${MATSDK_BUILD_APPLE_HTTP}
        -DBUILD_IOS=${MATSDK_BUILD_IOS}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME MSTelemetry CONFIG_PATH lib/cmake/MSTelemetry)

# Remove duplicate headers and empty dirs
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Install license
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

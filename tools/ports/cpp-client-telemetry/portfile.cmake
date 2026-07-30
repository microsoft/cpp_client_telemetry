# In-repo port validation (tests/vcpkg/*) sets MATSDK_VCPKG_SOURCE_DIR so the port
# builds the working tree under review instead of a pinned release -- this is what
# lets the port tests actually exercise the SDK source + manifest together. When
# the variable is unset (production installs), the pinned release is downloaded as
# usual, so the published port behavior is unchanged.
if(DEFINED ENV{MATSDK_VCPKG_SOURCE_DIR})
    set(SOURCE_PATH "$ENV{MATSDK_VCPKG_SOURCE_DIR}")
    if(NOT EXISTS "${SOURCE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR
            "MATSDK_VCPKG_SOURCE_DIR is set to '${SOURCE_PATH}', but no CMakeLists.txt "
            "was found there. It must point to a cpp_client_telemetry source checkout.")
    endif()
    message(STATUS "cpp-client-telemetry: building local source $ENV{MATSDK_VCPKG_SOURCE_DIR} (MATSDK_VCPKG_SOURCE_DIR is set)")
else()
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO microsoft/cpp_client_telemetry
        REF v3.10.173.1
        SHA512 e55bc35274236f57757660073c4dccccab3462342c8566212f1df4bf8824295a2bb3d3d79a11f3950e7c9252641827e9dd3d7c28c421dea3bdaee277e4f2ce32
        HEAD_REF main
    )
endif()

# Determine if Apple HTTP should be used (no curl needed).
# Note: MATSDK_BUILD_APPLE_HTTP must remain ON for macOS/iOS because the vcpkg.json
# curl dependency is excluded on these platforms.
set(MATSDK_BUILD_APPLE_HTTP OFF)
if(VCPKG_TARGET_IS_OSX OR VCPKG_TARGET_IS_IOS)
  set(MATSDK_BUILD_APPLE_HTTP ON)
endif()

# iOS build options
set(MATSDK_BUILD_IOS_LEGACY OFF)
if(VCPKG_TARGET_IS_IOS)
  set(MATSDK_BUILD_IOS_LEGACY ON)
endif()

# curl-openssl (default) and curl-mbedtls choose the TLS backend for the built-in
# HTTP client and are mutually exclusive. They only matter on Linux/Android: the
# curl dependency is platform-filtered to those triplets, so on Windows/macOS/iOS
# both features may be present (curl-openssl is a default) yet pull no curl, and
# the SDK uses WinInet / Apple HTTP there. vcpkg cannot express mutual exclusivity
# or "exactly one of", so validate it here -- but only where curl is actually used,
# to avoid failing legitimate cross-platform manifests on Windows/Apple.
set(_matsdk_http_features "")
foreach(_matsdk_http_feature curl-openssl curl-mbedtls)
  if(_matsdk_http_feature IN_LIST FEATURES)
    list(APPEND _matsdk_http_features ${_matsdk_http_feature})
  endif()
endforeach()
list(LENGTH _matsdk_http_features _matsdk_http_feature_count)
if(VCPKG_TARGET_IS_LINUX OR VCPKG_TARGET_IS_ANDROID)
  if(_matsdk_http_feature_count GREATER 1)
    message(FATAL_ERROR
      "curl-openssl (default) and curl-mbedtls are mutually exclusive but both were "
      "selected. To use mbedTLS, drop the defaults with the [core,...] form and "
      "re-select a SQLite backend (the [core,...] form also drops the default "
      "system-sqlite feature), e.g. "
      "cpp-client-telemetry[core,curl-mbedtls,system-sqlite] "
      "(or minimal-sqlite in place of system-sqlite).")
  elseif(_matsdk_http_feature_count EQUAL 0)
    # The built-in curl HTTP client requires exactly one TLS backend. The [core,...]
    # form drops the default curl-openssl, so fail fast (with a complete example)
    # rather than letting the SDK CMake fail later on a missing libcurl.
    message(FATAL_ERROR
      "On Linux/Android the built-in curl HTTP client requires exactly one TLS "
      "backend feature, but none was selected. The [core,...] form drops the "
      "default curl-openssl feature, so re-add a curl backend together with a "
      "SQLite backend, e.g. cpp-client-telemetry[core,curl-openssl,system-sqlite] "
      "(or curl-mbedtls / minimal-sqlite in place of those).")
  endif()
endif()

set(MATSDK_VCPKG_SQLITE_PROVIDER SYSTEM)
if("minimal-sqlite" IN_LIST FEATURES)
  set(MATSDK_VCPKG_SQLITE_PROVIDER MINIMAL)
endif()

if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
  set(MATSDK_VCPKG_BUILD_SHARED_LIBS ON)
else()
  set(MATSDK_VCPKG_BUILD_SHARED_LIBS OFF)
endif()

file(READ "${SOURCE_PATH}/CMakeLists.txt" MATSDK_ROOT_CMAKE)
set(MATSDK_PINNED_SOURCE_OPTIONS)
if(MATSDK_ROOT_CMAKE MATCHES "MATSDK_USE_VCPKG_DEPS")
  list(APPEND MATSDK_PINNED_SOURCE_OPTIONS -DMATSDK_USE_VCPKG_DEPS=ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${MATSDK_PINNED_SOURCE_OPTIONS}
        -DMATSDK_SQLITE_PROVIDER=${MATSDK_VCPKG_SQLITE_PROVIDER}
        -DBUILD_SHARED_LIBS=${MATSDK_VCPKG_BUILD_SHARED_LIBS}
        -DMATSDK_BUILD_HEADERS=ON
        -DMATSDK_BUILD_LIBRARY=ON
        -DMATSDK_BUILD_TEST_TOOL=OFF
        -DMATSDK_BUILD_UNIT_TESTS=OFF
        -DMATSDK_BUILD_FUNC_TESTS=OFF
        -DMATSDK_BUILD_JNI_WRAPPER=OFF
        -DMATSDK_BUILD_OBJC_WRAPPER=OFF
        -DMATSDK_BUILD_SWIFT_WRAPPER=OFF
        -DMATSDK_BUILD_PACKAGE=OFF
        -DBUILD_VERSION=${VERSION}
        -DMATSDK_BUILD_APPLE_HTTP=${MATSDK_BUILD_APPLE_HTTP}
        # Legacy aliases keep the pinned release fallback buildable until the
        # next release contains the canonical MATSDK_* options.
        -DBUILD_HEADERS=ON
        -DBUILD_LIBRARY=ON
        -DBUILD_TEST_TOOL=OFF
        -DBUILD_UNIT_TESTS=OFF
        -DBUILD_FUNC_TESTS=OFF
        -DBUILD_JNI_WRAPPER=OFF
        -DBUILD_OBJC_WRAPPER=OFF
        -DBUILD_SWIFT_WRAPPER=OFF
        -DBUILD_PACKAGE=OFF
        -DBUILD_APPLE_HTTP=${MATSDK_BUILD_APPLE_HTTP}
        -DBUILD_IOS=${MATSDK_BUILD_IOS_LEGACY}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME MSTelemetry CONFIG_PATH lib/cmake/MSTelemetry)

# Remove duplicate headers and empty dirs
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Install license
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

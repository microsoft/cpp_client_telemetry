# In-repo port validation (tests/vcpkg/*) sets MATSDK_VCPKG_SOURCE_DIR so the port
# builds the working tree under review instead of a pinned release -- this is what
# lets the port tests actually exercise the SDK source + manifest together. When
# the variable is unset (production installs), the pinned release is downloaded as
# usual, so the published port behavior is unchanged.
if(DEFINED ENV{MATSDK_VCPKG_SOURCE_DIR})
    set(SOURCE_PATH "$ENV{MATSDK_VCPKG_SOURCE_DIR}")
    message(STATUS "cpp-client-telemetry: building local source $ENV{MATSDK_VCPKG_SOURCE_DIR} (MATSDK_VCPKG_SOURCE_DIR is set)")
else()
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO microsoft/cpp_client_telemetry
        REF v3.10.161.1
        SHA512 4664b34ddce601d6a95669df4a59d11a6cc67de1f23de132192f791a275edc6a10b8498d340e6cf7d120d9e7a22c494d7517b24fc0954bf9e236e84a8800589a
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

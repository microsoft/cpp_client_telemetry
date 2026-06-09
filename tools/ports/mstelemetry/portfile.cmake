vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO microsoft/cpp_client_telemetry
    REF 65a0c754d2ee6a53bdcb3b79b36ac66483a7cf98
    SHA512 75da6be4233bfb05bafb1b31d6b80737e6efde4596adb93582dc0175f0154b8a5df7bdf5434d957ccdc31653da160959fe09f5e9f2997e6c0796759af9651755
    HEAD_REF main
)

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

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
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

# Install usage instructions
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

# Install license
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

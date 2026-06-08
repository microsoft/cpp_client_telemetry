vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO microsoft/cpp_client_telemetry
    REF 88c1302f6c2fea67f44bc10677c94581f28b5cfa
    SHA512 91d405f6d9504fcf529d0bdd045c01f06953e9309f657e92a303156baff8ada8065748b9a92e0c5f1ac47543e9ff77bcc22d856f8d637e990939bccc7e9c27bc
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

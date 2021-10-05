include(vcpkg_common_functions)

message("CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
message("CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")

if (UNIX)
    execute_process(COMMAND "${CMAKE_CURRENT_LIST_DIR}/get_repo_name.sh" OUTPUT_VARIABLE REPO_NAME ERROR_QUIET)
else()
    # execute_process(COMMAND git config --get remote.origin.url OUTPUT_VARIABLE REPO_URL ERROR_QUIET)
    # message("REPO_URL=${REPO_URL}")
    # string(REPLACE "/" ";" REPO_URL_LIST ${REPO_URL})
    # message(REPO_URL_LIST "list = ${REPO_URL_LIST}")
    # list(LENGTH ${REPO_URL_LIST} LAST_ITEM)
    # list(GET ${REPO_URL_LIST} ${LAST_ITEM} REPO_NAME)
endif()

message("REPO_NAME=${REPO_NAME}")

if (DEFINED REPO_NAME)
    # Use local snapshot since we already cloned the code
    get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
    message("Using local source snapshot from ${SOURCE_PATH}")
else()
    # Fetch from GitHub main
    message("Fetching source code from GitHub...")
    if (UNIX)
        vcpkg_from_github(
            OUT_SOURCE_PATH SOURCE_PATH
            REPO microsoft/cpp_client_telemetry
            HEAD_REF main
        )
    else()
        vcpkg_from_github(
            OUT_SOURCE_PATH SOURCE_PATH
            REPO microsoft/cpp_client_telemetry
            REF 4f60dd3bca305c2c0dd5ec2ed7b91d36b4de6dcf
            SHA512 9778df5aa65d95fe1d41739753495d29b3149676e98ac2e802a103604553f4f2b43bc2eb089c2e13dc695f70279287ea79ec6e2926fad03befe8a671f91d36fb
            HEAD_REF main
            PATCHES ${CMAKE_CURRENT_LIST_DIR}/v142-build.patch
        )
    endif()
endif()

# TODO: it will be slightly cleaner to perform pure CMake or Ninja build, by describing all possible variable options
# as separate triplets. Since we have a fairly non-trivial build logic in build.sh script - we use it as-is for now.
# build.sh itself should check if we are building under vcpkg and avoid installing deps that are coming from vcpkg.
if (UNIX)
    vcpkg_execute_build_process(
        COMMAND ${SOURCE_PATH}/build.sh noroot
        WORKING_DIRECTORY ${SOURCE_PATH}/
        LOGNAME build
    )

    vcpkg_execute_build_process(
        COMMAND ${SOURCE_PATH}/install.sh ${CURRENT_PACKAGES_DIR}
        WORKING_DIRECTORY ${SOURCE_PATH}/
        LOGNAME install
    )

    file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
else()
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

    vcpkg_install_msbuild(
        SOURCE_PATH ${SOURCE_PATH}
        PROJECT_SUBPATH Solutions/MSTelemetrySDK.sln
        LICENSE_SUBPATH LICENSE
        RELEASE_CONFIGURATION Release
        DEBUG_CONFIGURATION Debug
        OPTIONS /p:MATSDK_SHARED_LIB=1
        PLATFORM ${VCPKG_TARGET_ARCHITECTURE}
        PLATFORM_TOOLSET v142
        TARGET sqlite:Rebuild,win32-lib:Rebuild
        USE_VCPKG_INTEGRATION
    )

    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
    file(COPY "${SOURCE_PATH}/lib/include/public" DESTINATION "${CURRENT_PACKAGES_DIR}")
    file(RENAME "${CURRENT_PACKAGES_DIR}/public" "${CURRENT_PACKAGES_DIR}/include")
    file(COPY "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "Refer to LICENSE file")
endif()

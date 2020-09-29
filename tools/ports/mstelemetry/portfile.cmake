include(vcpkg_common_functions)

message("CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
message("CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")

execute_process(COMMAND "${CMAKE_CURRENT_LIST_DIR}/get_repo_name.sh" OUTPUT_VARIABLE REPO_NAME ERROR_QUIET)
message("REPO_NAME=${REPO_NAME}")

if (DEFINED REPO_NAME)
# Use local snapshot since we already cloned the code
get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
message("Using local source snapshot from ${SOURCE_PATH}")
else()
# Fetch from GitHub master
message("Fetching source code from GitHub...")
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO microsoft/cpp_client_telemetry
    HEAD_REF master
)
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
else()
# TODO: verify Windows build
vcpkg_execute_build_process(
    COMMAND ${SOURCE_PATH}/build-all.bat
    WORKING_DIRECTORY ${SOURCE_PATH}/
    LOGNAME build
)
endif()

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

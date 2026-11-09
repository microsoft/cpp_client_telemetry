cmake_minimum_required(VERSION 3.19)
get_filename_component(sdk_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
if(NOT DEFINED OUTPUT_DIR)
  set(OUTPUT_DIR "${sdk_root}/out/apple-embedding-options")
endif()
set(generator_args)
if(TEST_GENERATOR)
  list(APPEND generator_args -G "${TEST_GENERATOR}")
endif()
if(TEST_GENERATOR_PLATFORM)
  list(APPEND generator_args -A "${TEST_GENERATOR_PLATFORM}")
endif()

function(check_options apple vcpkg provided http class)
  # Keep MSBuild's nested compiler-probe paths below its legacy path limit.
  string(SHA1 name "${apple}-${vcpkg}-${provided}-${http}-${class}")
  string(SUBSTRING "${name}" 0 6 name)
  execute_process(COMMAND "${CMAKE_COMMAND}" ${generator_args}
    -S "${CMAKE_CURRENT_LIST_DIR}/embedding-options" -B "${OUTPUT_DIR}/${name}"
    "-DSDK_ROOT=${sdk_root}" "-DTEST_APPLE=${apple}" "-DTEST_VCPKG=${vcpkg}"
    "-DTEST_PROVIDED=${provided}" "-DTEST_HTTP=${http}"
    "-DMATSDK_APPLE_HTTP_DELEGATE_CLASS=${class}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(class STREQUAL "invalid-class")
    if(result EQUAL 0 OR NOT error MATCHES "must be a C identifier")
      message(FATAL_ERROR "Invalid delegate name was not rejected:\n${output}\n${error}")
    endif()
  elseif(NOT result EQUAL 0)
    message(FATAL_ERROR "${apple}/${vcpkg}/${provided}/${http}/${class} failed:\n${output}\n${error}")
  endif()
endfunction()

foreach(apple ON OFF)
  foreach(vcpkg ON OFF)
    foreach(provided ON OFF)
      check_options("${apple}" "${vcpkg}" "${provided}" ON MATStreamingSessionDelegate)
      check_options("${apple}" "${vcpkg}" "${provided}" ON ProductMATStreamingSessionDelegate)
    endforeach()
  endforeach()
endforeach()
check_options(ON ON OFF OFF ProductMATStreamingSessionDelegate)
check_options(ON ON OFF ON invalid-class)
message(STATUS "18 Apple dependency/delegate configurations passed")

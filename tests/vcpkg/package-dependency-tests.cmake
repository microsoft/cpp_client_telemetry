cmake_minimum_required(VERSION 3.19)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
if(NOT DEFINED OUTPUT_DIR)
  set(OUTPUT_DIR "${REPO_ROOT}/out/package-dependency-tests")
endif()

function(check_dependency_config sqlite_kind zlib_kind shared)
  string(MAKE_C_IDENTIFIER "${sqlite_kind}-${zlib_kind}-${shared}-${ARGN}" name)
  execute_process(
    COMMAND "${CMAKE_COMMAND}"
      -S "${REPO_ROOT}/tests/package-dependencies"
      -B "${OUTPUT_DIR}/${name}"
      "-DSDK_ROOT=${REPO_ROOT}"
      "-DSQLITE_KIND=${sqlite_kind}"
      "-DZLIB_KIND=${zlib_kind}"
      "-DSHARED=${shared}" ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(EXPECTED_MISSING_TARGET)
    if(result EQUAL 0 OR NOT error MATCHES "caller-provided ${EXPECTED_MISSING_TARGET}")
      message(FATAL_ERROR "${name} did not reject the missing provided target:\n${output}\n${error}")
    endif()
  elseif(NOT result EQUAL 0)
    message(FATAL_ERROR "${name} failed:\n${output}\n${error}")
  endif()
endfunction()

foreach(sqlite_kind PACKAGE PROVIDED APPLE_SYSTEM BUNDLED NONE)
  foreach(zlib_kind PACKAGE PROVIDED APPLE_SYSTEM BUNDLED)
    foreach(shared ON OFF)
      check_dependency_config("${sqlite_kind}" "${zlib_kind}" "${shared}")
    endforeach()
  endforeach()
endforeach()
check_dependency_config(PACKAGE PACKAGE OFF -DUNOFFICIAL_SQLITE=ON)
check_dependency_config(PROVIDED PROVIDED OFF -DLEGACY_SQLITE=ON)
foreach(shared ON OFF)
  check_dependency_config(PACKAGE PACKAGE "${shared}" -DCURL_KIND=PROVIDED)
endforeach()
foreach(target SQLite3::SQLite3 ZLIB::ZLIB CURL::libcurl)
  set(EXPECTED_MISSING_TARGET "${target}")
  check_dependency_config(PROVIDED PROVIDED OFF
    -DCURL_KIND=PROVIDED "-DMISSING_TARGET=${target}")
endforeach()

message(STATUS "All 44 package dependency configurations and 3 missing-target checks passed")

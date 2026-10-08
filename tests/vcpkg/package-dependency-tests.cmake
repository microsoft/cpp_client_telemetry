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
  elseif(EXPECTED_MISSING_PACKAGE)
    if(result EQUAL 0 OR NOT error MATCHES "${EXPECTED_MISSING_PACKAGE}")
      message(FATAL_ERROR "${name} did not reject the missing required package:\n${output}\n${error}")
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
foreach(shared ON OFF)
  foreach(openssl ON OFF)
    check_dependency_config(PACKAGE PACKAGE "${shared}"
      -DCURL_KIND=BUNDLED "-DOPENSSL=${openssl}")
  endforeach()
endforeach()
foreach(target SQLite3::SQLite3 ZLIB::ZLIB CURL::libcurl)
  set(EXPECTED_MISSING_TARGET "${target}")
  check_dependency_config(PROVIDED PROVIDED OFF
    -DCURL_KIND=PROVIDED "-DMISSING_TARGET=${target}")
endforeach()
unset(EXPECTED_MISSING_TARGET)

foreach(package SQLite3 unofficial-sqlite3 ZLIB CURL OpenSSL)
  set(options "-DMISSING_PACKAGE=${package}")
  if(package STREQUAL "unofficial-sqlite3")
    list(APPEND options -DUNOFFICIAL_SQLITE=ON)
  elseif(package STREQUAL "OpenSSL")
    list(APPEND options -DCURL_KIND=BUNDLED -DOPENSSL=ON)
  endif()
  foreach(mode OPTIONAL QUIET)
    foreach(shared ON OFF)
      check_dependency_config(PACKAGE PACKAGE "${shared}"
        ${options} "-DLOOKUP_MODE=${mode}")
    endforeach()
  endforeach()
  check_dependency_config(PACKAGE PACKAGE ON ${options} -DLOOKUP_MODE=REQUIRED)
  set(EXPECTED_MISSING_PACKAGE "${package}")
  check_dependency_config(PACKAGE PACKAGE OFF ${options} -DLOOKUP_MODE=REQUIRED)
  unset(EXPECTED_MISSING_PACKAGE)
endforeach()

message(STATUS "All 48 package dependency configurations, 3 missing-target checks, and 30 missing-package checks passed")

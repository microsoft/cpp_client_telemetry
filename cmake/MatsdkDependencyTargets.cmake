include_guard()
set(_MATSDK_DEPENDENCY_TARGETS_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(matsdk_add_interface_dependency target_name)
  if(ARGC LESS 2)
    message(FATAL_ERROR
      "matsdk_add_interface_dependency requires a target and at least one link dependency.")
  endif()

  if(NOT TARGET "${target_name}")
    add_library("${target_name}" INTERFACE IMPORTED GLOBAL)
  endif()
  set_property(TARGET "${target_name}" APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES "${ARGN}")
endfunction()

function(matsdk_add_package_system_dependency dependency_target canonical_target provider_value package_name)
  if(NOT "${provider_value}" STREQUAL "SYSTEM")
    return()
  endif()

  set(options APPLE_SYSTEM)
  set(one_value_args APPLE_LIBRARY LEGACY_TARGET)
  cmake_parse_arguments(MATSDK_PACKAGE_DEP "${options}" "${one_value_args}" "" ${ARGN})

  if(MATSDK_PACKAGE_DEP_APPLE_SYSTEM)
    if(NOT DEFINED MATSDK_PACKAGE_DEP_APPLE_LIBRARY
        OR MATSDK_PACKAGE_DEP_APPLE_LIBRARY STREQUAL "")
      message(FATAL_ERROR
        "APPLE_LIBRARY is required for Apple system dependencies.")
    endif()
    include("${_MATSDK_DEPENDENCY_TARGETS_DIR}/MatsdkAppleSystemDeps.cmake")
    matsdk_add_apple_system_library(
      "${canonical_target}" "${MATSDK_PACKAGE_DEP_APPLE_LIBRARY}")
  elseif(NOT TARGET "${canonical_target}")
    if(DEFINED MATSDK_PACKAGE_DEP_LEGACY_TARGET
       AND TARGET "${MATSDK_PACKAGE_DEP_LEGACY_TARGET}")
      matsdk_add_interface_dependency(
        "${canonical_target}" "${MATSDK_PACKAGE_DEP_LEGACY_TARGET}")
    else()
      find_dependency(${package_name})
    endif()
  endif()

  if(NOT TARGET "${canonical_target}"
     AND DEFINED MATSDK_PACKAGE_DEP_LEGACY_TARGET
     AND TARGET "${MATSDK_PACKAGE_DEP_LEGACY_TARGET}")
    matsdk_add_interface_dependency(
      "${canonical_target}" "${MATSDK_PACKAGE_DEP_LEGACY_TARGET}")
  endif()
  if(NOT TARGET "${canonical_target}")
    message(FATAL_ERROR
      "${package_name} did not create the required ${canonical_target} target.")
  endif()

  matsdk_add_interface_dependency("${dependency_target}" "${canonical_target}")
endfunction()

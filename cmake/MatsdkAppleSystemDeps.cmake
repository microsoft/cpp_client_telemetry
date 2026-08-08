# Apple ships system SQLite and zlib but no CMake package config for either, so
# there is no find_package() to call. This defines the canonical imported
# target as a thin wrapper around the raw linker library name (e.g. "sqlite3",
# "z").
#
# This file is shared between the root CMakeLists.txt (build time) and the
# installed MSTelemetryConfig.cmake (consume time, via install(FILES...) in
# lib/CMakeLists.txt) so the two never drift out of sync -- in particular the
# GLOBAL keyword below, which is required so a consumer that calls
# find_package(MSTelemetry) in one directory can link MSTelemetry::mat from a
# sibling/non-descendant directory.
function(matsdk_add_apple_system_library target_name library_name)
  if(NOT TARGET "${target_name}")
    add_library("${target_name}" INTERFACE IMPORTED GLOBAL)
    set_property(TARGET "${target_name}" PROPERTY
      INTERFACE_LINK_LIBRARIES "${library_name}")
  endif()
endfunction()

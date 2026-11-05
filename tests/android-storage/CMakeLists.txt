cmake_minimum_required(VERSION 3.19...3.31)
project(AndroidStorageSelectionTests LANGUAGES C CXX)

if(NOT ANDROID OR NOT DEFINED EXPECT_ROOM)
  message(FATAL_ERROR "Configure with the Android toolchain and -DEXPECT_ROOM=ON or OFF.")
endif()

add_subdirectory(../../lib/android_build/maesdk/src/main/cpp aar)

if((EXPECT_ROOM AND NOT MATSDK_ANDROID_USE_ROOM)
    OR (NOT EXPECT_ROOM AND MATSDK_ANDROID_USE_ROOM))
  message(FATAL_ERROR "The AAR wrapper overrode the selected storage backend.")
endif()
get_target_property(sources mat SOURCES)
get_target_property(definitions matsdk_internal_config INTERFACE_COMPILE_DEFINITIONS)
if(EXPECT_ROOM)
  if(NOT "offline/OfflineStorage_Room.cpp" IN_LIST sources
      OR "offline/OfflineStorage_SQLite.cpp" IN_LIST sources
      OR NOT "USE_ROOM" IN_LIST definitions)
    message(FATAL_ERROR "Room storage sources and compile definitions do not agree.")
  endif()
else()
  if(NOT "offline/OfflineStorage_SQLite.cpp" IN_LIST sources
      OR "offline/OfflineStorage_Room.cpp" IN_LIST sources
      OR "USE_ROOM" IN_LIST definitions
      OR NOT TARGET SQLite3::SQLite3)
    message(FATAL_ERROR "SQLite storage sources, dependency, and compile definitions do not agree.")
  endif()
endif()
message(STATUS "AAR storage selection passed: Room=${MATSDK_ANDROID_USE_ROOM}")

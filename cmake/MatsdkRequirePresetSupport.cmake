cmake_minimum_required(VERSION 3.15)

if(CMAKE_VERSION VERSION_LESS 3.21)
  message(FATAL_ERROR
    "The 1DS build wrappers require CMake 3.21 or newer for CMakePresets.json "
    "support. Direct CMake builds retain the CMake 3.15 minimum.")
endif()

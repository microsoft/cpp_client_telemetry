set(_MATSDK_DEFAULT_INSTALL_DIR "/usr/local")
if(NOT "$ENV{MATSDK_INSTALL_DIR}" STREQUAL "")
  set(_MATSDK_DEFAULT_INSTALL_DIR "$ENV{MATSDK_INSTALL_DIR}")
endif()

set(MATSDK_INSTALL_DIR "${_MATSDK_DEFAULT_INSTALL_DIR}" CACHE PATH "MSTelemetry install prefix")
set(MATSDK_INCLUDE_DIR "${MATSDK_INSTALL_DIR}/include/mat" CACHE PATH "MSTelemetry public headers")
set(MATSDK_LIB_DIR "${MATSDK_INSTALL_DIR}/lib" CACHE PATH "MSTelemetry library directory")

if(NOT EXISTS "${MATSDK_LIB_DIR}/libmat.a"
   AND NOT EXISTS "${MATSDK_LIB_DIR}/libmat.dylib"
   AND EXISTS "${MATSDK_LIB_DIR}/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/libmat.a")
  set(MATSDK_LIB_DIR "${MATSDK_LIB_DIR}/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu" CACHE PATH "MSTelemetry library directory" FORCE)
endif()

find_library(MATSDK_LIBRARY NAMES mat HINTS "${MATSDK_LIB_DIR}" NO_DEFAULT_PATH)
if(NOT MATSDK_LIBRARY)
  message(FATAL_ERROR "Could not find libmat under ${MATSDK_LIB_DIR}. Set MATSDK_INSTALL_DIR or MATSDK_LIB_DIR.")
endif()

if(NOT EXISTS "${MATSDK_INCLUDE_DIR}")
  message(FATAL_ERROR "Could not find mat headers under ${MATSDK_INCLUDE_DIR}. Set MATSDK_INSTALL_DIR or MATSDK_INCLUDE_DIR.")
endif()

set(MATSDK_SAMPLE_INCLUDE_DIRS "${MATSDK_INCLUDE_DIR}")

set(MATSDK_SAMPLE_PLATFORM_LIBS "")
if(APPLE)
  list(APPEND MATSDK_SAMPLE_PLATFORM_LIBS
    "-framework CoreFoundation"
    "-framework Foundation"
    "-framework CFNetwork"
    "-framework Network"
    "-framework SystemConfiguration"
  )
  if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    list(APPEND MATSDK_SAMPLE_PLATFORM_LIBS "-framework UIKit")
  else()
    list(APPEND MATSDK_SAMPLE_PLATFORM_LIBS "-framework IOKit")
  endif()
endif()

find_library(MATSDK_SQLITE3_LIB NAMES sqlite3 HINTS "${MATSDK_INSTALL_DIR}/lib" NO_DEFAULT_PATH)
if(NOT MATSDK_SQLITE3_LIB)
  set(MATSDK_SQLITE3_LIB sqlite3)
endif()

mark_as_advanced(MATSDK_INSTALL_DIR MATSDK_INCLUDE_DIR MATSDK_LIB_DIR MATSDK_LIBRARY MATSDK_SQLITE3_LIB)

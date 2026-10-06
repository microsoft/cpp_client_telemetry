# SDK-owned policy stays private to SDK targets and out of dependency builds.
add_library(matsdk_build_options INTERFACE)
if(MSVC)
  target_compile_options(matsdk_build_options INTERFACE
    /W4
    $<$<BOOL:${MATSDK_WARNINGS_AS_ERRORS}>:/WX>
    /Gy
    $<$<CXX_COMPILER_ID:MSVC>:/Gw>)
else()
  target_compile_options(matsdk_build_options INTERFACE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-but-set-variable
    $<$<BOOL:${MATSDK_WARNINGS_AS_ERRORS}>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:-Wno-unknown-warning-option>
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-unknown-warning-option>
    $<$<CONFIG:Debug>:-ggdb>
    $<$<CONFIG:Debug>:-gdwarf-2>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-fno-builtin-malloc>
    $<$<CONFIG:Debug>:-fno-builtin-calloc>
    $<$<CONFIG:Debug>:-fno-builtin-realloc>
    $<$<CONFIG:Debug>:-fno-builtin-free>
    $<$<NOT:$<CONFIG:Debug>>:-Os>
    $<$<NOT:$<CONFIG:Debug>>:-fmerge-all-constants>
    -ffunction-sections
    $<$<CXX_COMPILER_ID:GNU,Clang>:-fdata-sections>)
  if(NOT WIN32)
    target_compile_options(matsdk_build_options INTERFACE
      -fvisibility=hidden
      $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)
  endif()
endif()

if(MATSDK_DISABLE_EXCEPTIONS)
  if(MSVC)
    target_compile_options(matsdk_build_options INTERFACE
      $<$<COMPILE_LANGUAGE:CXX>:/EHs-c->
      $<$<COMPILE_LANG_AND_ID:CXX,Clang>:/clang:-fno-exceptions>)
    target_compile_definitions(matsdk_build_options INTERFACE
      $<$<COMPILE_LANGUAGE:CXX>:_HAS_EXCEPTIONS=0>)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    target_compile_options(matsdk_build_options INTERFACE
      $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fno-exceptions>)
  else()
    message(FATAL_ERROR
      "MATSDK_DISABLE_EXCEPTIONS is unsupported with ${CMAKE_CXX_COMPILER_ID}.")
  endif()
endif()

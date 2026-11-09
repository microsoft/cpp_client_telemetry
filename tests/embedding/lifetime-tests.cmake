# Static embedding lets the instrumented production objects replace their archive counterparts.
if(BUILD_SHARED_LIBS)
  return()
endif()

set(lifetime_dir "${CMAKE_CURRENT_BINARY_DIR}/lifetime-sdk")
file(MAKE_DIRECTORY "${lifetime_dir}")
foreach(source pal/PAL.cpp api/AuthTokensController.cpp)
  set(source_path "${sdk_source}/lib/${source}")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${source_path}")
  file(READ "${source_path}" instrumented)
  if(source STREQUAL "pal/PAL.cpp")
    string(REPLACE "static const std::string sdkVersion(" "static const LifetimeString sdkVersion("
      instrumented "${instrumented}")
    string(REPLACE "using SdkVersion = std::string;" "using SdkVersion = LifetimeString;"
      instrumented "${instrumented}")
    # Hook entry, not a local variable: the allocation-free implementation has none.
    string(REGEX REPLACE "(PlatformAbstractionLayer::getSdkVersion\\(\\) const[ \r\n]*\\{)"
      "\\1\n        LifetimeVersionAccessed();" instrumented "${instrumented}")
    if(NOT instrumented MATCHES "LifetimeVersionAccessed\\(\\);")
      message(FATAL_ERROR "Update version accessor lifetime instrumentation")
    endif()
    string(PREPEND instrumented
      "extern \"C\" void LifetimeVersionAccessed();\n")
    set(hook "LifetimeVersionDestroyed")
    set(type String)
    set(base string)
  else()
    string(REPLACE "static std::mutex mutex;" "static LifetimeMutex mutex;"
      instrumented "${instrumented}")
    string(REPLACE "using AuthMutex = std::mutex;" "using AuthMutex = LifetimeMutex;"
      instrumented "${instrumented}")
    set(hook "LifetimeMutexDestroyed")
    set(type Mutex)
    set(base mutex)
  endif()
  if(NOT instrumented MATCHES "(LifetimeVersionAccessed\\(\\);|static LifetimeMutex mutex|using AuthMutex = LifetimeMutex)")
    message(FATAL_ERROR "Update lifetime instrumentation for ${source}")
  endif()
  string(PREPEND instrumented
    "#include <string>\n#include <mutex>\n"
    "extern \"C\" void ${hook}();\n"
    "struct Lifetime${type} : std::${base} {\n"
    "  using std::${base}::${base};\n"
    "  ~Lifetime${type}() { ${hook}(); }\n"
    "};\n")
  get_filename_component(name "${source}" NAME)
  file(WRITE "${lifetime_dir}/${name}" "${instrumented}")
endforeach()

add_executable(sdk_lifetime_tests "${CMAKE_CURRENT_LIST_DIR}/lifetime-tests.cpp"
  "${lifetime_dir}/PAL.cpp" "${lifetime_dir}/AuthTokensController.cpp")
target_link_libraries(sdk_lifetime_tests PRIVATE MSTelemetry::mat matsdk_internal_config matsdk_build_options)
target_compile_features(sdk_lifetime_tests PRIVATE cxx_std_17)
target_include_directories(sdk_lifetime_tests PRIVATE "$<TARGET_PROPERTY:mat,INCLUDE_DIRECTORIES>"
  "${sdk_source}/lib/api")
add_dependencies(embedding_test sdk_lifetime_tests)
foreach(mode version mutex both)
  add_test(NAME sdk_lifetime_${mode} COMMAND sdk_lifetime_tests "${mode}")
  set_tests_properties(sdk_lifetime_${mode} PROPERTIES TIMEOUT 30)
endforeach()

# Embed the actual archive, not the instrumented copies, to catch unload-time leaks.
add_library(sdk_version_reload_fixture SHARED version-reload-fixture.cpp)
target_link_libraries(sdk_version_reload_fixture PRIVATE MSTelemetry::mat
  matsdk_internal_config matsdk_build_options)
target_compile_features(sdk_version_reload_fixture PRIVATE cxx_std_17)
target_include_directories(sdk_version_reload_fixture PRIVATE
  "$<TARGET_PROPERTY:mat,INCLUDE_DIRECTORIES>")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # GNU-unique symbols pin DSOs; symbol interposition can bind to another SDK copy.
  target_compile_options(mat PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fno-gnu-unique>)
  target_link_options(sdk_version_reload_fixture PRIVATE
    "-Wl,--exclude-libs,ALL" "-Wl,-Bsymbolic")
endif()

add_executable(sdk_version_reload_test version-reload-test.cpp)
target_link_libraries(sdk_version_reload_test PRIVATE ${CMAKE_DL_LIBS}
  matsdk_internal_config matsdk_build_options)
target_include_directories(sdk_version_reload_test PRIVATE
  "$<TARGET_PROPERTY:mat,INCLUDE_DIRECTORIES>")
add_dependencies(embedding_test sdk_version_reload_fixture sdk_version_reload_test)
add_test(NAME sdk_version_reload
  COMMAND sdk_version_reload_test "$<TARGET_FILE:sdk_version_reload_fixture>")
set_tests_properties(sdk_version_reload PROPERTIES TIMEOUT 30)

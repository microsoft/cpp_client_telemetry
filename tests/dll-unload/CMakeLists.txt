add_library(debug-listener-unload-module MODULE debug-listener-unload-module.cpp)
target_link_libraries(debug-listener-unload-module PRIVATE mat matsdk_internal_config)
target_include_directories(debug-listener-unload-module PRIVATE "${PROJECT_SOURCE_DIR}/lib")

# The host must not link the SDK: its only reference is the explicitly loaded DLL.
add_executable(debug-listener-unload-test debug-listener-unload-test.cpp)
target_compile_definitions(debug-listener-unload-test PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
add_dependencies(debug-listener-unload-test debug-listener-unload-module)
foreach(mode IN ITEMS idle dispatch)
  add_test(NAME debug-listener-unload-${mode}
    COMMAND debug-listener-unload-test
      $<TARGET_FILE:debug-listener-unload-module> ${mode}
    CONFIGURATIONS Debug)
  set_tests_properties(debug-listener-unload-${mode} PROPERTIES TIMEOUT 30)
endforeach()

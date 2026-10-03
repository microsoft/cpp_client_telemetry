add_library(debug-listener-unload-module MODULE debug-listener-unload-module.cpp)
target_link_libraries(debug-listener-unload-module PRIVATE mat matsdk_internal_config)
target_include_directories(debug-listener-unload-module PRIVATE
  "${PROJECT_SOURCE_DIR}/lib"
  "${PROJECT_SOURCE_DIR}/lib/include"
  "${PROJECT_SOURCE_DIR}/lib/include/public"
  "${PROJECT_SOURCE_DIR}/lib/include/mat"
  "${PROJECT_SOURCE_DIR}/tests")

# The host must not link the SDK: its only reference is the explicitly loaded DLL.
add_executable(debug-listener-unload-test debug-listener-unload-test.cpp)
target_compile_definitions(debug-listener-unload-test PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
add_dependencies(debug-listener-unload-test debug-listener-unload-module)
foreach(mode IN ITEMS idle dispatch network-native network-disabled network-legacy network-legacy-no-cost network-legacy-failures)
  add_test(NAME debug-listener-unload-${mode}
    COMMAND debug-listener-unload-test
      $<TARGET_FILE:debug-listener-unload-module> ${mode}
    CONFIGURATIONS Debug)
  set_tests_properties(debug-listener-unload-${mode} PROPERTIES TIMEOUT 30)
endforeach()
foreach(mode IN ITEMS network-native network-disabled network-legacy network-legacy-no-cost network-legacy-failures)
  set_tests_properties(debug-listener-unload-${mode} PROPERTIES SKIP_RETURN_CODE 77)
endforeach()

add_executable(network-detector-reload-test network-detector-reload-test.cpp)
target_compile_definitions(network-detector-reload-test PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(network-detector-reload-test PRIVATE ole32)
add_dependencies(network-detector-reload-test debug-listener-unload-module)
foreach(mode IN ITEMS native disabled legacy legacy-no-cost legacy-failures)
  add_test(NAME network-detector-reload-${mode}
    COMMAND network-detector-reload-test $<TARGET_FILE:debug-listener-unload-module> ${mode}
    CONFIGURATIONS Debug)
  set_tests_properties(network-detector-reload-${mode} PROPERTIES TIMEOUT 30 SKIP_RETURN_CODE 77)
endforeach()

if(EXISTS $ENV{ANT_HOME})
  set(ANT_HOME $ENV{ANT_HOME})
  message(STATUS "Ant found in ${ANT_HOME} (environment variable is set).")
elseif(EXISTS ${CMAKE_SOURCE_DIR}/skype-ant)
  set(ANT_HOME ${CMAKE_SOURCE_DIR}/skype-ant)
  message(STATUS "Ant found in ${ANT_HOME} (unpacked in project root).")
elseif(EXISTS /opt/skype-ant)
  set(ANT_HOME /opt/skype-ant)
  message(STATUS "Ant found in /opt/skype-ant (QB or proper installation).")
elseif(EXISTS ${CMAKE_SOURCE_DIR}/../../skype-ant)
  set(ANT_HOME ${CMAKE_SOURCE_DIR}/../../skype-ant)
  message(STATUS "Ant found in ${ANT_HOME} (Orient).")
else()
  message(SEND_ERROR "Ant not found.")
endif()

if(ORIENT)
  set(ANDROID_SDK_ROOT ${ORIENT_TOOLS_ANDROID-SDK})
else()
  set(ANDROID_SDK_ROOT ${SYSROOT}/android/android-sdk-linux)
endif()

add_custom_target(aria_jni_wrapper
  ALL
  VERBATIM
  COMMAND
    ${ANT_HOME}/bin/ant
    -f ${CMAKE_CURRENT_SOURCE_DIR}/build-jars.xml
    -Dconf_kernos=android
    -Dqb.configId=${QB_CONFIGID}
    -Djava_src_dir=${CMAKE_CURRENT_SOURCE_DIR}
    -Dbuild_dir=${CMAKE_CURRENT_BINARY_DIR}
    -Dandroid_sdk_path=${ANDROID_SDK_ROOT}/platforms/android-24
    build-jni-jar-${QB_FLAVOR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/telemetryclient-${QB_FLAVOR}.jar
  DESTINATION
    jni
)

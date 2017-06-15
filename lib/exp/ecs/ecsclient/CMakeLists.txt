cmake_policy(SET CMP0004 OLD)

if (${CMAKE_BUILD_TYPE} MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    set(BUILD_TYPE_FOLDER "debug")
elseif(${CMAKE_BUILD_TYPE} MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(BUILD_TYPE_FOLDER "release")
endif()

set(ECSCLIENT ECSClient)

if (USE_GCOV)
    set(GCOV_LIB gcov)
endif()

if (MSVC)
    if (RT_WINRT)
        message("Add link path for winrt arm")
        set(PLATFORM_DEPENDENT_LINK_LIBRARIES "msxml6.lib ${AUF_LIB_EXTRA_DEPENDS}")
    else()
        message("Add link path for windows")
        set(PLATFORM_DEPENDENT_LINK_LIBRARIES "wininet")
    endif()
endif()

# Setup sources
set(ECSCLIENT_SOURCE
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSClientConfig.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSClientUtils.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSClientUtils.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSConfigCache.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSConfigCache.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSClient.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ECSClient.cpp
     
     ${CMAKE_CURRENT_SOURCE_DIR}/../../include/IECSClient.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../common/JsonHelper.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../common/JsonHelper.cpp
    )
    
add_library(${ECSCLIENT} STATIC 
    ${ECSCLIENT_SOURCE}
)    
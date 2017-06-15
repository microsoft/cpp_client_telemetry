cmake_policy(SET CMP0004 OLD)

if (${CMAKE_BUILD_TYPE} MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    set(BUILD_TYPE_FOLDER "debug")
elseif(${CMAKE_BUILD_TYPE} MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(BUILD_TYPE_FOLDER "release")
endif()

set(AFDCLIENT AFDClient)

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
set(AFDCLIENT_SOURCE
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDClientConfig.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDClientUtils.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDClientUtils.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDConfigCache.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDConfigCache.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDClient.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/AFDClient.cpp
     
     ${CMAKE_CURRENT_SOURCE_DIR}/../../include/IAFDClient.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../common/JsonHelper.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../common/JsonHelper.cpp
    )
    
add_library(${AFDCLIENT} STATIC 
    ${AFDCLIENT_SOURCE}
)    
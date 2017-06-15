include_directories("../include")
include_directories("../internal")
include_directories("../common/")
include_directories("${SYSROOT}/gmock/include")

if (__MAC__ OR __IOS__)
    add_subdirectory(ecsclient)
endif()

if ((${SCT_CONF_ID} MATCHES "win.*") OR (${SCT_CONF_ID} MATCHES "uap.*"))
    add_subdirectory(ecsclient)
endif()

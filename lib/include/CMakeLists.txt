string(REPLACE "." ";" MATSDK_BUILD_VERSION_SPLIT "${MATSDK_BUILD_VERSION}")
list(GET MATSDK_BUILD_VERSION_SPLIT 0 BUILD_VERSION_MAJOR)
list(GET MATSDK_BUILD_VERSION_SPLIT 1 BUILD_VERSION_MINOR)
list(GET MATSDK_BUILD_VERSION_SPLIT 2 BUILD_VERSION_PATCH)
list(GET MATSDK_BUILD_VERSION_SPLIT 3 BUILD_NUMBER)

#TODO: allow regeneration of Version.hpp using template
#configure_file(public/Version.hpp.template "${CMAKE_CURRENT_SOURCE_DIR}/public/Version.hpp")

set(MATSDK_PUBLIC_HEADER_INSTALL_EXCLUDES
  PATTERN "*.template" EXCLUDE
)

if(MATSDK_USE_VCPKG_DEPS)
  # GitHub source archives used by the public vcpkg port do not include the
  # private lib/modules submodule, so do not install public headers whose
  # exported factories/functions are implemented only by those modules.
  list(APPEND MATSDK_PUBLIC_HEADER_INSTALL_EXCLUDES
    PATTERN "CompliantByDefaultFilterApi.hpp" EXCLUDE
    PATTERN "IAFDClient.hpp" EXCLUDE
    PATTERN "ICdsFactory.hpp" EXCLUDE
    PATTERN "IECSClient.hpp" EXCLUDE
  )
endif()

install(
  DIRECTORY
    public/
  DESTINATION
    include/mat
  ${MATSDK_PUBLIC_HEADER_INSTALL_EXCLUDES}
)

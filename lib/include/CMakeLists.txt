string(REPLACE "." ";" MATSDK_BUILD_VERSION_SPLIT "${MATSDK_BUILD_VERSION}")
list(GET MATSDK_BUILD_VERSION_SPLIT 0 BUILD_VERSION_MAJOR)
list(GET MATSDK_BUILD_VERSION_SPLIT 1 BUILD_VERSION_MINOR)
list(GET MATSDK_BUILD_VERSION_SPLIT 2 BUILD_VERSION_PATCH)
list(GET MATSDK_BUILD_VERSION_SPLIT 3 BUILD_NUMBER)

#TODO: allow regeneration of Version.hpp using template
#configure_file(public/Version.hpp.template "${CMAKE_CURRENT_SOURCE_DIR}/public/Version.hpp")

install(
  DIRECTORY
    public/
  DESTINATION
    include/mat
  PATTERN "public/*.template" EXCLUDE
)

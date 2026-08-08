include(FetchContent)

function(matsdk_configure_fetched_static_target target_name)
  if(NOT TARGET "${target_name}")
    message(FATAL_ERROR "Fetched dependency target not found: ${target_name}")
  endif()
  set_target_properties("${target_name}" PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden)
  target_compile_options("${target_name}" PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,GNU,Clang>:-ffunction-sections;-fdata-sections>)
endfunction()

function(matsdk_fetch_curl out_target)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR
      "MATSDK_CURL_PROVIDER=FETCH is currently supported only on Linux. "
      "Use MATSDK_CURL_PROVIDER=SYSTEM for this platform.")
  endif()
  if(TARGET CURL::libcurl)
    message(FATAL_ERROR
      "MATSDK_CURL_PROVIDER=FETCH requires owning the CURL::libcurl target, "
      "but a target with that name already exists. Use MATSDK_CURL_PROVIDER=SYSTEM.")
  endif()

  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)

  foreach(option IN ITEMS
      BUILD_SHARED_LIBS
      BUILD_TESTING
      ENABLE_PROGRAMS
      ENABLE_TESTING
      GEN_FILES
      UNSAFE_BUILD
      INSTALL_MBEDTLS_HEADERS
      MBEDTLS_FATAL_WARNINGS
      USE_SHARED_MBEDTLS_LIBRARY
      LINK_WITH_PTHREAD
      BUILD_CURL_EXE
      BUILD_EXAMPLES
      BUILD_LIBCURL_DOCS
      BUILD_MISC_DOCS
      ENABLE_CURL_MANUAL
      CURL_ENABLE_EXPORT_TARGET
      CURL_USE_OPENSSL
      CURL_USE_PKGCONFIG
      CURL_USE_CMAKECONFIG
      CURL_ZLIB
      CURL_BROTLI
      CURL_ZSTD
      USE_LIBIDN2
      CURL_USE_LIBPSL
      CURL_USE_LIBSSH2
      CURL_USE_LIBSSH
      CURL_USE_GSSAPI
      CURL_USE_GSASL
      USE_NGHTTP2
      USE_NGTCP2
      USE_QUICHE
      ENABLE_ARES
      ENABLE_UNIX_SOCKETS)
    set(${option} OFF)
  endforeach()

  foreach(option IN ITEMS
      BUILD_STATIC_LIBS
      DISABLE_PACKAGE_CONFIG_AND_INSTALL
      CURL_DISABLE_INSTALL
      HTTP_ONLY
      CURL_DISABLE_ALTSVC
      CURL_DISABLE_HSTS
      CURL_DISABLE_COOKIES
      CURL_DISABLE_NETRC
      CURL_DISABLE_MIME
      CURL_DISABLE_DOH
      CURL_DISABLE_AWS
      CURL_DISABLE_BEARER_AUTH
      CURL_DISABLE_DIGEST_AUTH
      CURL_DISABLE_KERBEROS_AUTH
      CURL_DISABLE_NEGOTIATE_AUTH)
    set(${option} ON)
  endforeach()

  if(MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "MBEDTLS")
    set(USE_STATIC_MBEDTLS_LIBRARY ON)
    set(CURL_USE_MBEDTLS ON)
    set(MBEDTLS_CONFIG_FILE "")
    set(MBEDTLS_USER_CONFIG_FILE "")

    FetchContent_Declare(
      matsdk_mbedtls
      URL ${MATSDK_MBEDTLS_URL}
      URL_HASH SHA256=${MATSDK_MBEDTLS_SHA256})
    FetchContent_MakeAvailable(matsdk_mbedtls)

    foreach(target mbedtls mbedx509 mbedcrypto)
      matsdk_configure_fetched_static_target("${target}")
    endforeach()

    set(MBEDTLS_INCLUDE_DIR "${matsdk_mbedtls_SOURCE_DIR}/include")
    set(MBEDTLS_LIBRARY mbedtls)
    set(MBEDX509_LIBRARY mbedx509)
    set(MBEDCRYPTO_LIBRARY mbedcrypto)
    set(MBEDTLS_USE_STATIC_LIBS ON)
    foreach(_matsdk_mbedtls_target mbedtls mbedx509 mbedcrypto)
      if(TARGET ${_matsdk_mbedtls_target}
         AND NOT TARGET MbedTLS::${_matsdk_mbedtls_target})
        add_library(MbedTLS::${_matsdk_mbedtls_target}
          ALIAS ${_matsdk_mbedtls_target})
      endif()
    endforeach()
  elseif(MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "OPENSSL")
    set(CURL_USE_OPENSSL ON)
    find_package(OpenSSL REQUIRED)
  endif()

  FetchContent_Declare(
    matsdk_curl
    URL ${MATSDK_CURL_URL}
    URL_HASH SHA256=${MATSDK_CURL_SHA256})
  FetchContent_MakeAvailable(matsdk_curl)

  if(NOT TARGET CURL::libcurl OR NOT TARGET libcurl_static)
    message(FATAL_ERROR "The embedded static CURL::libcurl target was not created.")
  endif()

  matsdk_configure_fetched_static_target(libcurl_static)

  set(_matsdk_fetched_curl_targets libcurl_static)
  if(MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "MBEDTLS")
    list(APPEND _matsdk_fetched_curl_targets mbedtls mbedx509 mbedcrypto)
    foreach(_matsdk_mbedtls_support_target everest p256m)
      if(TARGET ${_matsdk_mbedtls_support_target})
        list(APPEND _matsdk_fetched_curl_targets
          ${_matsdk_mbedtls_support_target})
      endif()
    endforeach()
  endif()
  set(MATSDK_FETCHED_CURL_TARGETS
    "${_matsdk_fetched_curl_targets}" PARENT_SCOPE)
  set(${out_target} CURL::libcurl PARENT_SCOPE)
endfunction()

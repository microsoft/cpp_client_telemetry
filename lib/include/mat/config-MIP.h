#define EVTSDK_VERSION_PREFIX "EVT"
#if defined(_WIN32)
#define HAVE_MAT_UTC
#endif
#if defined(HAVE_PRIVATE_MODULES)
/* #define HAVE_MAT_EXP */
#define HAVE_MAT_FIFOSTORAGE
#endif
#define HAVE_MAT_JSONHPP
#define HAVE_MAT_ZLIB
#define HAVE_MAT_LOGGING
#define HAVE_MAT_STORAGE
/* #define HAVE_MAT_DEFAULT_HTTP_CLIENT */
#if defined(_WIN32) && !defined(_WINRT_DLL)
#define HAVE_MAT_NETDETECT
#endif

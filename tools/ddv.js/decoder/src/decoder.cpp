#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <zlib.h>

using namespace std;

// Temporary stub for now. We don't need this C API function because we're not linking
// the SDK in decoder, but the default mat/config.h requires it. TODO: get rid of it
#include <mat.h>
EVTSDK_LIBABI evt_status_t EVTSDK_LIBABI_CDECL evt_api_call_default(evt_context_t*)
{
    return (evt_status_t)0;
}
// end of temporary stub

int main(int argc, char** argv)
{
#ifdef EMSCRIPTEN
    const char* buff = "Test string: emscripten";
#else
    const char* buff = "Test string: native";
#endif
    printf("%s\n", buff);
    auto crc = crc32(0, (const Bytef*)buff, strlen(buff));
    printf("crc32=%lu\n", crc);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include <zlib.h>

#include "jslog.hpp"
#include "stubs.hpp"

#include "PayloadDecoder.hpp"

using namespace std;

#if EMSCRIPTEN

#include <emscripten.h>

/**
 * Notify JS when decoded payload result is ready
 */
EMSCRIPTEN_KEEPALIVE
extern "C" void onRequestDecoded(char* buffer, size_t size)
{
    printf("onDecodeResult: 0x%p[%zu]", buffer, size);
    fflush(stdout);
    //
    EM_ASM({onRequestDecoded($0, $1)}, buffer, size);
}

/**
 * Function invoked to decode request body
 */
EMSCRIPTEN_KEEPALIVE
extern "C" void decodeRequest(char* contentType, char* contentEncoding, char* buffer, size_t size)
{
    printf("Content-Type: %s\n", contentType);
    printf("Content-Encoding: %s\n", contentEncoding);
    printf("Body: %p (%zu bytes)\n", buffer, size);
    fflush(stdout);

    // TODO: lookup the matching decoder for Content-Type and Content-Encoding.
    // Currently we support only Common Schema 3.0 / Bond with gzip compression.
    std::string result;
    std::vector<uint8_t> body(reinterpret_cast<uint8_t*>(buffer), reinterpret_cast<uint8_t*>(buffer) + size);
    MAT::exporters::DecodeRequest(body, result, true);
    onRequestDecoded((char*)(result.c_str()), result.length());
}

static bool isRunning = true;

/**
 * Stop the Module instance
 */
EMSCRIPTEN_KEEPALIVE
extern "C" void stop()
{
    isRunning = false;
}

void main_loop()
{
    static unsigned seq = 0;

#if 0
    // Uncomment to debug EM hearbeats:
    printf("EM heartbeat: %u\n", seq);
    fflush(stdout);
#endif

    // THIS COMMENT IS LEFT AS A REMINDER: NEVER SLEEP IN THIS LOOP!
    // If you sleep here for too long, you're stealing time from JS
    // worker threads... So... NEVER... EVER... SLEEP HERE!!!
    // std::this_thread::sleep_for(std::chrono::seconds(2));

    seq++;
    if (!isRunning)
    {
        // Notify Javascript that we're about to exit
        console_log("Stopping em_main...");
        EM_ASM({onEmDone()});
    }
}

/**
 * Emscripten logic
 */
int em_main(int argc, char** argv)
{
    // Notify Javascript that we're ready
    EM_ASM({onEmInit()});
    console_log("Starting em_main...");
    emscripten_set_main_loop(main_loop, 10, 0);
    return 0;
}
#endif

/**
 * TODO: support compiling natively as CGI module OR as node microservice with STDIN/STDOUT
 * https://en.wikipedia.org/wiki/Common_Gateway_Interface
 */
std::thread worker;

int main(int argc, char** argv)
{
#ifdef EMSCRIPTEN
    // std::thread{ em_main, argc, argv }.detach();
    return em_main(argc, argv);
#else
    return 0;
#endif
}

#if EMSCRIPTEN

#include <string>

#include <emscripten.h>

/**
 * Send const char* string to console.log
 */
EM_JS(void, console_log, (const char* str), { console.log(UTF8ToString(str)); });

/**
 * Send std::string to console_log wrapper using zero-copy
 */
EMSCRIPTEN_KEEPALIVE
void console_log_string(const std::string& msg)
{
    // Call JS function using EM_ASM
    EM_ASM({console_log($0, $1)}, msg.c_str(), msg.length());
}

/**
 * Send std::string to console_log wrapper using strdup
 */
EMSCRIPTEN_KEEPALIVE
void console_log_strdup(const std::string& msg)
{
    char* s = strdup(msg.c_str());
    // Call JS function using EM_ASM
    EM_ASM({console_log($0, $1)}, s, msg.length());
    free(s);
}

#endif

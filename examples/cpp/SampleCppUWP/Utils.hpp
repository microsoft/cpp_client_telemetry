#pragma once

#define MAX_BUFF_SIZE       65535

/**
 * This routine is not thread-safe!
 */
static void DebugPrintf(const char *fmt, ...)
{
    static char szBuffer[MAX_BUFF_SIZE];
    va_list args;
    va_start(args, fmt);
    int nBuf;
    nBuf = _vsnprintf_s(szBuffer, MAX_BUFF_SIZE-1, fmt, args);
    ::OutputDebugStringA(szBuffer);
    ::OutputDebugStringA("\n"); // eppend endl
    va_end(args);
}

#define TraceMsg DebugPrintf

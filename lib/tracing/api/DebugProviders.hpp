//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DEBUGLOGGERPROVIDERS_HPP
#define DEBUGLOGGERPROVIDERS_HPP

#ifdef USE_WINCRYPT
#include <sha1-wincrypt.hpp>
#else
extern "C" {
#include "sha1.h"
}
#endif

/// <summary>
/// This class must reside in NS_DBG namespace.
/// </summary>
class DebugLoggerStreamBuffer : public std::streambuf {};

/// <summary>
/// Log to file
/// </summary>
class LogFileStream : public std::ofstream {

public:

    /// <summary>
    /// Reopen existing LogFileStream with a new path
    /// </summary>
    /// <param name="filename"></param>
    /// <param name="mode"></param>
    void reopen(const char* filename, ios_base::openmode mode = ios_base::out)
    {
        if (is_open())
        {
            close();
        }
        open(filename, mode);
        // We might as well check if the failbit is set
        if (!is_open())
        {
            // If file cannot be created, keep logging to /dev/null
            open(DBG_LOG_NULL);
        }
    }

};

/// <summary>
/// Base abstract class that requires to override xsputn method
/// </summary>
class DebugStreamBuffer : public std::streambuf {
public:
    DebugStreamBuffer() : std::streambuf() {};
    virtual std::streamsize xsputn(const char* s, std::streamsize n) = 0;
};

/// <summary>
/// Convenience template to simplify implementation of custom debug streams
/// </summary>
template <class T>
class DebugStringStream : public std::ostream {
protected:
    T buffer;
public:
    DebugStringStream<T>() : std::ostream(&buffer) {};
};

#ifdef _WIN32
/// <summary>
/// Transport layer implementation for OutputDebugStringA.
/// This function does not natively support Unicode debug,
/// but Unicode debug strings may be passed down as UTF-8.
/// </summary>
class OutputDebugStringStreamBuffer : public DebugStreamBuffer {
public:
    /// <summary>
    /// Pass UTF-8 string down to OutputDebugStringA
    /// </summary>
    /// <param name="s"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        OutputDebugStringA(s);
        return n;
    }
};

/**
* Convert Windows GUID type to string
*/
#define MAX_GUID_LEN    36
static std::string GuidToString(GUID *guid)
{
    char buff[MAX_GUID_LEN + 1] = { 0 };
    int rc = snprintf(buff, sizeof(buff),
        "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
    (rc); // If something is not right, then the string is going to be empty.
    return std::string((const char*)(buff));
}

/// <summary>
/// Output stream wrapper for OutputDebugString
/// </summary>
class OutputDebugStringStream : public DebugStringStream<OutputDebugStringStreamBuffer> {};

/// <summary>
/// Transport layer implementation for ETW.
/// This function converts UTF-8 strings to wide-strings.
/// </summary>
class ETWStringStreamBuffer : public DebugStreamBuffer {

protected:
    const REGHANDLE INVALID_HANDLE = _UI64_MAX;

    /// <summary>
    /// ETW handle
    /// </summary>
    REGHANDLE handle;

    /// <summary>
    /// Convert UTF-8 string to UTF-8 wide string
    /// </summary>
    /// <param name="in"></param>
    /// <returns></returns>
    inline std::wstring to_utf16_string(const std::string& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        return converter.from_bytes(in);
    }

public:

    /// <summary>
    /// Register event provider with ETW by path (string GUID)
    /// </summary>
    /// <param name="ProviderId"></param>
    /// <returns></returns>
    REGHANDLE open(const char* path)
    {
        GUID guid;
        if (UuidFromStringA((unsigned char __RPC_FAR *)path, &guid) != 0)
            return INVALID_HANDLE;
        return open(guid);
    }

    /// <summary>
    /// Register event provider with ETW by GUID
    /// </summary>
    /// <param name="ProviderId"></param>
    /// <returns></returns>
    REGHANDLE open(const GUID& path)
    {
        if (EventRegister(&path, NULL, NULL, &handle) != ERROR_SUCCESS)
        {
            // There was an error registering the ETW provider
            handle = INVALID_HANDLE;
        }
        return handle;
    }

    /// <summary>
    /// Unregister ETW handle 
    /// </summary>
    void close()
    {
        if (handle != INVALID_HANDLE)
        {
            EventUnregister(handle);
            handle = INVALID_HANDLE;
        }
    }

    /// <summary>
    /// Convert DebugLevel prefix to ETW level
    /// </summary>
    /// <param name="c"></param>
    /// <returns></returns>
    static inline char toLevel(char c)
    {
        switch (c) {
        case 'D':
        case 'T':
            c = 0x5; // Verbose
            break;
        case 'I':
            c = 0x4; // Informational
            break;
        case 'W':
            c = 0x3; // Warning
            break;
        case 'E':
            c = 0x2; // Error
            break;
        case 'F':
            c = 0x1; // Critical
            break;
        default:
            c = 0;  // LogAlways
            // We log all events of unknown level
        }
        return c;
    }

    /// <summary>
    /// Pass string down to EventWriteString.
    /// </summary>
    /// <param name="s"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        // Ideally the string is expected to be UTF-16, but downstream processing
        // tools don't care. In fact Microsoft Message Analyzer shows you the ASCII
        // string buffers just fine - it has a special mode for that. Also ASCII
        // strings are x2 times more compact than UTF-16. The caller ensures that
        // that the end of each buffer is always padded with at least two zeros
        // (UTF-16 NUL byte), so that the kernel processor of these ETW events
        // does not choke.
#if 0
        // Enable this code to pass down string as UTF-16 instead of ASCII
        auto ws = to_utf16_string(s);
        wchar_t *buff = ws.c_str();
#else
        char *buff = (char *)s;
#endif

        if (handle != INVALID_HANDLE)
        {
            // Use first byte as a level hint
            UCHAR level = toLevel(buff[0]);
            buff += 2;
            EventWriteString(handle, level, 0, (PCWSTR)buff);
        }
        return n;
    }
};

/// <summary>
/// Class to send EventPayload to ETW provider
/// </summary>
class ETWStringStream : public DebugStringStream<ETWStringStreamBuffer> {

protected:

    /// <summary>
    /// Open ETW provider buffer
    /// </summary>
    /// <param name="filename">ETW provider GUID string</param>
    /// <param name="mode"></param>
    virtual void open(const char* filename, ios_base::openmode mode = ios_base::out)
    {
        buffer.open(filename);
    }


    /// <summary>
    /// Open ETW provider buffer
    /// </summary>
    /// <param name="filename">ETW provider GUID string</param>
    /// <param name="mode"></param>
    virtual void open(GUID guid, ios_base::openmode mode = ios_base::out)
    {
        buffer.open(guid);
    }

    /// <summary>
    /// Close (unregister) ETW provider buffer
    /// </summary>
    virtual void close()
    {
        buffer.close();
    }

public:

    /// <summary>
    /// Transform ETW provider name to provider GUID as described here:
    /// https://blogs.msdn.microsoft.com/dcook/2015/09/08/etw-provider-names-and-guids/
    /// </summary>
    /// <param name="providerName"></param>
    /// <returns></returns>
    static GUID GetProviderGuid(const char* providerName)
    {
        std::string name(providerName);
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);

        size_t len = name.length() * 2 + 0x10;
        uint8_t *buffer = new uint8_t[len];
        uint32_t num = 0x482c2db2;
        uint32_t num2 = 0xc39047c8;
        uint32_t num3 = 0x87f81a15;
        uint32_t num4 = 0xbfc130fb;

        for (int i = 3; i>=0; i--)
        {
            buffer[i] = (uint8_t)num;
            num = num >> 8;
            buffer[i + 4] = (uint8_t)num2;
            num2 = num2 >> 8;
            buffer[i + 8] = (uint8_t)num3;
            num3 = num3 >> 8;
            buffer[i + 12] = (uint8_t)num4;
            num4 = num4 >> 8;
        }

        for (size_t j = 0; j < name.length(); j++)
        {
            buffer[((2 * j) + 0x10) + 1] = (uint8_t)name[j];
            buffer[(2 * j) + 0x10] = (uint8_t)(name[j] >> 8);
        }

        const size_t sha1_hash_size = 21;
        uint8_t *buffer2 = new uint8_t[sha1_hash_size];
#ifdef USE_WINCRYPT
        /**
        * WinCrypt works only for Win32 Desktop applications.
        * We can't use it in a generic platform-agnostic way
        */
        DWORD len2 = sha1_hash_size;
        ::WinCryptHelper::sha1((const BYTE*)buffer, (DWORD)len, (BYTE*)buffer2, len2);
#else
        SHA1((char *)buffer2, (const char *)buffer, len);
#endif
        unsigned long a = (((((buffer2[3] << 8) + buffer2[2]) << 8) + buffer2[1]) << 8) + buffer2[0];
        unsigned short b = (unsigned short)((buffer2[5] << 8) + buffer2[4]);
        unsigned short num9 = (unsigned short)((buffer2[7] << 8) + buffer2[6]);

        GUID guid;
        guid.Data1 = a;
        guid.Data2 = b;
        guid.Data3 = (unsigned short)((num9 & 0xfff) | 0x5000);
        guid.Data4[0] = buffer2[8];
        guid.Data4[1] = buffer2[9];
        guid.Data4[2] = buffer2[10];
        guid.Data4[3] = buffer2[11];
        guid.Data4[4] = buffer2[12];
        guid.Data4[5] = buffer2[13];
        guid.Data4[6] = buffer2[14];
        guid.Data4[7] = buffer2[15];

        delete buffer;
        delete buffer2;

        return guid;
    }


    /// <summary>
    /// Specifies the string representation of ETW provider GUID to log to
    /// </summary>
    /// <param name="filename"></param>
    ETWStringStream(const char* providerName) : DebugStringStream()
    {
        if (providerName[0] == '{')
        {
            // It's a GUID passed down as a string. We literally convert it
            // from string to GUID and open ETW provider by GUID.
            open(providerName);
            return;
        }

        // Assume that we're dealing with a provider name:
        // - we generate a GUID hash using SHA-1
        // - we open ETW provider using that generated hash
        // Converting Provider names to ETW GUIDs is described here:
        /// https://blogs.msdn.microsoft.com/dcook/2015/09/08/etw-provider-names-and-guids/
        GUID guid = GetProviderGuid(providerName);
        open(guid);
    }

    /// <summary>
    /// Specifies the string representation of ETW provider GUID to log to
    /// </summary>
    /// <param name="filename"></param>
    ETWStringStream(GUID guid) : DebugStringStream()
    {
        open(guid);
    }

    virtual ~ETWStringStream()
    {
        close();
    }

};

#endif

#endif


// Copyright (c) Microsoft. All rights reserved.
#include "FileUtils.hpp"
#include "Utils.hpp"

#include <string>
#include <fstream>
#include <streambuf>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// Return file size by filename
    /// </summary>
    /// <param name="filename">Filename</param>
    /// <returns>File size</returns>
    size_t FileGetSize(const char* filename)
    {
#ifdef _WIN32
        std::wstring filename_w = to_utf16_string(filename);
        std::ifstream in(filename_w.c_str(), std::ifstream::ate | std::ifstream::binary);
#else
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
#endif
        return (size_t)(in.tellg());
    }

    int FileDelete(const char* filename)
    {
        int result = 0;
#ifndef _WIN32
        /* All OS other than Windows support UTF-8...   */
        result = std::remove(filename);
#else
        /* ...but Windows requires us to support UTF-16 */
        std::wstring filename_w = to_utf16_string(filename);
        result = ::DeleteFileW(filename_w.c_str());
#endif
        return result;
    }

    std::FILE* FileOpen(const char* filename, const char *mode)
    {
#ifdef _WIN32
        FILE* result = nullptr;
        std::wstring  path = to_utf16_string(filename);
        std::wstring _mode = to_utf16_string(mode);
        _wfopen_s(&result, path.c_str(), _mode.c_str());
        return result;
#else
        return std::fopen(filename, mode);
#endif
    }

    int FileClose(std::FILE* _Stream)
    {
        return std::fclose(_Stream);
    }

    /**
     * Read file contents into std::string.
     *
     * @param       flename
     * @return      File contents
     */
    std::string FileGetContents(const char *filename)
    {
#ifdef _WIN32
        std::ifstream t(to_utf16_string(filename));
#else
        std::ifstream t(filename);
#endif
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        return str;
    }

    bool FileWrite(const char* filename, const char* contents)
    {
#ifdef _WIN32
        std::ofstream t(to_utf16_string(filename), std::ofstream::out | std::ofstream::trunc);
#else
        std::ofstream t(filename, std::ofstream::out | std::ofstream::trunc);
#endif
        if (t.is_open())
        {
            t << contents;
            return true;
        }
        return false;
    }

    bool FileExists(const char* name) {
#ifdef _WIN32
        std::ifstream t(to_utf16_string(name));
#else
        std::ifstream t(name);
#endif
        return t.good();
    }

} ARIASDK_NS_END

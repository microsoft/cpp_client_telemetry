//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "FileUtils.hpp"
#include "Utils.hpp"

#include <string>
#include <fstream>
#include <streambuf>

namespace MAT_NS_BEGIN
{

    /**
     * Return file size by filename.
     *
     * @param       filename    UTF-8 filename
     * @return      File size
     */
    size_t FileGetSize(const char* filename)
    {
#ifdef _WIN32
#ifndef _WINRT
        /* Use Win32 Desktop API to get file name. Cannot rely on ifstream(wstring) if built with libc++ instead of msvcrt++ */
        LARGE_INTEGER largeInt;
        std::wstring filename_w = to_utf16_string(filename);
        HANDLE hFile = CreateFileW(filename_w.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        GetFileSizeEx(hFile, &largeInt);
        CloseHandle(hFile);
        return (size_t)(largeInt.QuadPart);
#else
        /* Building for WinRT / UWP requires msvcrt++ runtime - CreateFileW API is 'Desktop only' */
        std::wstring filename_w = to_utf16_string(filename);
        std::ifstream in(filename_w.c_str(), std::ifstream::ate | std::ifstream::binary);
        return (size_t)(in.tellg());
#endif
#else
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return (size_t)(in.tellg());
#endif
    }

    /**
     * Delete file.
     *
     * @param       filename    UTF-8 flename
     * @return      0 upon success or non-zero value on error.
     */
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

    /**
     * Open file stream.
     *
     * @param       filename    UTF-8 filename
     * @param       mode        File mode:
     *
     * @return      File stream on success, null pointer on failure.
     */
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

    /**
     * Close file stream.
     *
     * @param       _Stream    File stream
     * @return      0 on success, EOF otherwise
     */
    int FileClose(std::FILE* _Stream)
    {
        return std::fclose(_Stream);
    }

    /**
     * Read file contents into std::string.
     *
     * @param       UTF-8 flename
     * @return      File contents. Supports only UTF-8 or ASCII-encoded text files.
     */
    std::string FileGetContents(const char *filename)
    {
#ifdef _WIN32
        char buff[256] = { 0 };
        std::string result;
        FILE* fp = FileOpen(filename, "r");
        if (fp != nullptr)
        {
            while (fgets(buff, sizeof(buff), fp))
            {
                result += buff;
            }
            FileClose(fp);
        }
        return result;
#else
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        return str;
#endif
    }

    /**
     * Writes the C string pointed by contents to the file named filename (UTF-8).
     *
     * @param       flename    UTF-8 filename
     * @param       contents   File contents in UTF-8 or ASCII
     *
     * @return      true on success, false on failure
     */
    bool FileWrite(const char* filename, const char* contents)
    {
#ifdef _WIN32
        bool result = false;
        FILE* fp = FileOpen(filename, "w+");
        if (fp != nullptr)
        {
            result = (fputs(contents, fp) >= 0);
            FileClose(fp);
        }
        return result;
#else
        std::ofstream t(filename, std::ofstream::out | std::ofstream::trunc);
        if (t.is_open())
        {
            t << contents;
            return true;
        }
        return false;
#endif
    }

    /**
     * Check if file exists.
     *
     * @param       name    UTF-8 file name
     * @return      File contents
     */
    bool FileExists(const char* name)
    {
#ifdef _WIN32
        std::wstring filename_w = to_utf16_string(name);
        DWORD dwAttrib = GetFileAttributesW(filename_w.c_str());
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
        std::ifstream t(name);
        return t.good();
#endif
    }

} MAT_NS_END


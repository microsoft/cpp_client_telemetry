//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
/*
 * sysinfo_sources.cpp
 *
 *  Created on: Nov 3, 2017
 *      Author: Max Golovanov <maxgolov@microsoft.com>
 */

#include "sysinfo_sources_impl.hpp"
#include "pal/PAL.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <string.h>

#include <sstream>
#include <fstream>
#include <streambuf>
#include <list>

#include <unistd.h>
#include <sys/utsname.h>

#include <iostream>
#include <iomanip>

#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>

#include <algorithm>

#include "EventProperty.hpp"

#if defined(__linux__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for tm_gmtoff and tm_zone */
#endif
#include <time.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#include <libgen.h>
#include "TargetConditionals.h"
#include "sysinfo_utils_apple.hpp"

#if defined(TARGET_MAC_OS) && !defined(MATSDK_DISABLE_DEVICE_ID)

#include <IOKit/IOKitLib.h>

// This would be better than  int gethostuuid(uuid_t id, const struct timespec *wait);
void get_platform_uuid(char * buf, int bufSize)
{
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);
    CFStringGetCString(uuidCf, buf, bufSize, kCFStringEncodingMacRoman);
    CFRelease(uuidCf);
}

#endif // TARGET_MAC_OS

std::string get_app_name()
{
    using PAL::getMATSDKLogComponent;
    std::vector<char> appId(PATH_MAX+1, 0);
    uint32_t length = static_cast<uint32_t>(appId.size());
    if(_NSGetExecutablePath(&appId[0], &length))
    {
        if (length == 0)
        {
            LOG_WARN("Executable path length is empty");
            return {};
        }
        appId.resize(length, 0);
        if (_NSGetExecutablePath(&appId[0], &length) != 0)
        {
            LOG_WARN("Unable to read executable path");
            return {};
        }
    }
    std::string result = basename(appId.data());
    return result;
}
#endif

/**
 * Read file contents
 *
 * @param filename
 * @return
 */
inline std::string ReadFile(const char *filename, sysinfo_selector selector)
{
    using PAL::getMATSDKLogComponent;
    auto mode = std::ios::in;
    if (selector == sysinfo_selector::first_null)
        mode |= std::ios::binary;
    std::ifstream input(filename, mode);
    std::string result;
    char ch;
    while (input.get(ch))
    {
        if ((selector == sysinfo_selector::first_null && ch == '\0') ||
            (selector == sysinfo_selector::first_line && ch == '\n'))
        {
            return result;
        }
        if (selector == sysinfo_selector::first_null && result.size() == MAX_COMMAND_LINE_SIZE)
        {
            LOG_WARN("Command-line executable exceeds %zu bytes; truncating", MAX_COMMAND_LINE_SIZE);
            result.push_back(ch);
            size_t length = MAX_COMMAND_LINE_SIZE;
            // Avoid cutting a UTF-8 sequence at the command-line boundary.
            while (length > 0 && (static_cast<unsigned char>(result[length]) & 0xc0) == 0x80)
                --length;
            result.resize(length);
            return result;
        }
        result.push_back(ch);
    }
    if (input.bad())
    {
        LOG_WARN("Unable to read system information source");
        return {};
    }
    return result;
}

/**
 * Execute command and get output
 * @param cmd Command to execute
 * @return output
 */
#if !defined(__APPLE__) && !defined(MATSDK_DISABLE_DEVICE_ID)
static std::string Exec(const char* cmd)
{
    using PAL::getMATSDKLogComponent;
    std::array<char, 128> buffer;
    std::string result;
    auto close_pipe = [](FILE* file) { pclose(file); };
    std::unique_ptr<FILE, decltype(close_pipe)> pipe(popen(cmd, "r"), close_pipe);
    if (!pipe)
    {
        LOG_WARN("Unable to execute system information command");
        return result;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        const size_t count = strlen(buffer.data());
        result.append(buffer.data(), count);
    }
    if (ferror(pipe.get()))
    {
        LOG_WARN("Unable to read system information command output");
        return {};
    }
    // Remove EOL. In all use-cases below we don't need it.
    if (!result.empty() && result[result.length()-1]=='\n')
    {
        result.erase(result.length()-1);
    }

    return result;
}
#endif

/**
 * Read a node value, select it without regex and store it in cache
 *
 * @param key       Field name
 * @return          true if field value is found and saved in cache
 */
bool sysinfo_sources::fetch(std::string key)
{
    using PAL::getMATSDKLogComponent;

    for (auto& kv : (*this))
    {
        if (kv.first != key)
            continue;
        if (kv.second.selector == sysinfo_selector::key_value && kv.second.name == nullptr)
        {
            LOG_WARN("System information key-value selector is missing its name");
            continue;
        }
        std::string contents = ReadFile(kv.second.path, kv.second.selector);
        if (kv.second.selector != sysinfo_selector::key_value)
        {
            cache[key] = std::move(contents);
            return true;
        }
        std::istringstream lines(contents);
        std::string line;
        const std::string prefix = std::string(kv.second.name) + "=";
        while (std::getline(lines, line))
        {
            if (line.compare(0, prefix.size(), prefix) != 0)
                continue;
            std::string value = line.substr(prefix.size());
            if (!value.empty() && value.back() == '\r')
                value.pop_back();
            if (value.size() >= 2 &&
                ((value.front() == '"' && value.back() == '"') ||
                 (value.front() == '\'' && value.back() == '\'')))
            {
                value = value.substr(1, value.size() - 2);
            }
            cache[key] = std::move(value);
            return true;
        }
    }
    return false;
}

/**
 * Add source descriptor to multimap.
 *
 * @param key
 * @param val
 */
void sysinfo_sources::add(const std::string& key, const sysinfo_source_t& val)
{
    (*this).insert(std::pair<std::string, sysinfo_source_t>(key, val));
}

/**
 * Static configuration provisioning for where to fetch the props from
 */
sysinfo_sources::sysinfo_sources() :
        std::multimap<std::string, sysinfo_source_t>()
{
}

/**
 * Retrieve value by key from sysinfo_sources. Try to fetch from cache,
 * if not found, then fetch from filesystem and save to in-ram cache.
 *
 * @param key
 * @return
 */
const std::string& sysinfo_sources::get(std::string key)
{
    if(cache.find(key) == cache.end())
        fetch(key);
    return cache[key];
}

/**
 * Obtain system hardware and application information
 */
sysinfo_sources_impl::sysinfo_sources_impl() : sysinfo_sources()
{
    struct utsname buf;
    uname(&buf);
#if defined(__linux__)
    // Obtain Linux system information from filesystem
#ifndef MATSDK_DISABLE_DEVICE_ID
    add("devId", { "/etc/machine-id", sysinfo_selector::raw});
#endif
    add("osName", {"/etc/os-release", sysinfo_selector::key_value, "ID"});
    add("osVer", {"/etc/os-release", sysinfo_selector::key_value, "VERSION_ID"});
    add("osRel", {"/etc/os-release", sysinfo_selector::key_value, "VERSION"});
    add("osBuild", {"/proc/version", sysinfo_selector::first_line});

    time_t t = time(NULL);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"  // error: missing initializer for member �tm::tm_min� [-Werror=missing-field-initializers]
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"  // error: missing initializer for member �tm::tm_min� [-Werror=missing-field-initializers]
#endif

    struct tm lt { 0 };
    localtime_r(&t, &lt);

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    int hh = lt.tm_gmtoff / 3600;
    int mm = (lt.tm_gmtoff / 60) % 60;
    std::ostringstream oss;
    oss << ((hh<0)?"-":"+"); // +hh:mm or -hh:mm
    oss << std::setw(2) << std::setfill('0') << std::abs(hh);
    oss << std::setw(1) << ":";
    oss << std::setw(2) << std::setfill('0') << std::abs(mm);
    cache["tz"] = oss.str();
#endif

#if defined(__MINGW32__) || defined(__MSYS__)
    // Obtain MinGW Device ID from registry
#ifndef MATSDK_DISABLE_DEVICE_ID
    add("devId",    { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/ComputerHardwareId", sysinfo_selector::raw});
#endif
    add("devMake",  { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/SystemManufacturer", sysinfo_selector::raw});
    add("devModel", { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/SystemProductName", sysinfo_selector::raw});
#endif

#if defined(__APPLE__)
    cache["devMake"] = "Apple";
    cache["devModel"] = GetDeviceModel();
    cache["osName"] = GetDeviceOsName();
    cache["osVer"] = GetDeviceOsVersion();
    cache["osRel"] = GetDeviceOsRelease();
    cache["osBuild"] = GetDeviceOsBuild();
    cache["devClass"] = GetDeviceClass();

    // Populate user timezone as hh:mm offset from UTC timezone. Example for PST: "-08:00"
    CFTimeZoneRef tz = CFTimeZoneCopySystem();
    CFTimeInterval minsFromGMT = CFTimeZoneGetSecondsFromGMT(tz, CFAbsoluteTimeGetCurrent()) / 60.0;
    CFRelease(tz);
    std::ostringstream oss;
    int hh = std::abs((int)minsFromGMT / 60);
    int mm = std::abs((int)minsFromGMT % 60);
    if (minsFromGMT<0)
    {
        oss << "-";
    }
    oss << std::setw(2) << std::setfill('0') << hh;
    oss << std::setw(1) << ":";
    oss << std::setw(2) << std::setfill('0') << mm;
    cache["tz"] = oss.str();
#endif

    // Fallback to uname if above methods failed
    if (!get("osVer").compare(""))
    {
        cache["osVer"]  = (const char *)(buf.version);
    }

    if (!get("osName").compare(""))
    {
        cache["osName"] = (const char *)(buf.sysname);
    }

    if (!get("osRel").compare(""))
    {
        cache["osRel"]  = (const char *)(buf.release);
    }

#ifndef __APPLE__
    add("appId", {"/proc/self/cmdline", sysinfo_selector::first_null});
#else
    cache["appId"] = get_app_name();
#endif

#ifndef MATSDK_DISABLE_DEVICE_ID
    if (!get("devId").compare(""))
    {
#ifdef __APPLE__
        std::string contents = GetDeviceId();
#if TARGET_OS_IPHONE
        cache["devId"] = "i:";
#else
        cache["devId"] = "u:";
#endif // TARGET_OS_IPHONE
        cache["devId"] += MAT::GUID_t(contents.c_str()).to_string();
#else
        // We were unable to obtain Device Id using standard means.
        // Try to use hash of blkid + hostname instead. Both blkid
        // and hostname would rarely change, as well as guarantee
        // at least some protection from cloned VM images.
        std::string contents = Exec("echo `blkid; hostname`");
        if (!contents.empty())
        {
            uint8_t guid_bytes[16] = { 0 };
            for(size_t i=0; i<contents.length(); i++)
            {   // Simple XOR of contents to generate a UUID
                guid_bytes[i % 16] ^= contents.at(i);
            }
            cache["devId"] = MAT::GUID_t(guid_bytes).to_string();
        }
#endif
    }
#endif

}

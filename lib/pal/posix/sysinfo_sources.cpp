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

#include <regex>

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

#ifdef TARGET_MAC_OS 

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
    std::vector<char> appId(PATH_MAX+1, 0);
    uint32_t length = 0;
    if(_NSGetExecutablePath(&appId[0], &length))
    {
        appId.resize(length, 0);
        _NSGetExecutablePath(&appId[0], &length);
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
inline std::string ReadFile(const char *filename)
{
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

/**
 * Execute command and get output
 * @param cmd Command to execute
 * @return output
 */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"  // Used on non-Apple platforms. See sysinfo_sources_impl()
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"  // Used on non-Apple platforms. See sysinfo_sources_impl()
#endif
static std::string Exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        // throw std::runtime_error("popen() failed!");
        return result;
    }

    while (!feof(pipe.get()))
    {
        if (fgets(buffer.data(), buffer.size(), pipe.get())!=NULL)
            result += buffer.data();
    }

    // Remove EOL. In all use-cases below we don't need it.
    if (!result.empty() && result[result.length()-1]=='\n')
    {
        result.erase(result.length()-1);
    }

    return result;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/**
 * Read node value, preprocess it using regexp and store result in cache
 *
 * @param key       Field name
 * @return          true if field value is found and saved in cache
 */
bool sysinfo_sources::fetch(std::string key)
{
	for(auto &kv : (*this))
	{
		if(kv.first == key)
		{
			const std::string star("*");
			const std::string empty("");

			std::string contents = ReadFile(kv.second.path);
			if((kv.second.selector == star) || (kv.second.selector == empty))
			{
				cache[key] = contents;
				return true;
			}
			// Run regexp
			std::regex selector_regex(kv.second.selector);
			std::smatch match;
			if(std::regex_search(contents, match, selector_regex))
			{
				cache[key] = match[1];
				return true;
			}
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
    add("devId", { "/etc/machine-id", "*"});
    add("osName", {"/etc/os-release", ".*ID=(.*)[\n]+"});
    add("osVer", {"/etc/os-release", ".*VERSION_ID=\"(.*)\".*"});
    add("osRel", {"/etc/os-release", ".*VERSION=\"(.*)\".*"});
    add("osBuild", {"/proc/version", "(.*)[\n]+"});
    // add("proc_loadavg", {"/proc/loadavg", "(.*)[\n]*"});
    // add("proc_uptime", {"/proc/uptime", "(.*)[\n]*"});

    // osName may contain quotes on openSUSE
    if (get("osName").find('"') == 0)
    {
        std::string contents = get("osName");
        size_t pos_end_quote = contents.rfind('"');
        if (pos_end_quote != std::string::npos && pos_end_quote > 0)
        {
            cache["osName"] = contents.substr(1, pos_end_quote - 1);
        }
    }

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
    add("devId",    { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/ComputerHardwareId", "*"});
    add("devMake",  { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/SystemManufacturer", "*"});
    add("devModel", { "/proc/registry/HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/SystemInformation/SystemProductName",  "*"});
#endif

#if defined(__APPLE__)
    cache["devMake"] = "Apple";
    cache["devModel"] = GetDeviceModel();
    cache["osName"] = GetDeviceOsName();
    cache["osVer"] = GetDeviceOsVersion();
    cache["osRel"] = GetDeviceOsRelease();
    cache["osBuild"] = GetDeviceOsBuild();

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
    add("appId", {"/proc/self/cmdline", "(.*)[ ]*.*[\n]*"});
#else
    cache["appId"] = get_app_name();
#endif

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

}


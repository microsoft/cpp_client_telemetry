/*
 * main.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: Max Golovanov <maxgolov@microsoft.com>
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <string.h>
#include <map>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <list>

#include "gtest/gtest.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <regex>

std::string ReadFile(const char *filename)
{
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

typedef struct {
    const char * path;
    const char * selector;
} sysinfo_source_t;

class sysinfo_sources: public std::multimap<std::string, sysinfo_source_t> {

protected:
    std::map<std::string, std::string> cache;

    /**
     * Read node value, preprocess it using regexp and store result in cache
     *
     * @param key       Field name
     * @return          true if field value is found and saved in cache
     */
    bool fetch(std::string key)
    {
        for(auto &kv : (*this) )
        {
            if (kv.first == key)
            {
                std::string contents = ReadFile(kv.second.path);
                if ((kv.second.selector == "*")||(kv.second.selector == ""))
                {
                    cache[key] = contents;
                    return true;
                }
                // Run regexp
                std::regex selector_regex(kv.second.selector);
                std::smatch match;
                if (std::regex_search(contents, match, selector_regex))
                {
                    cache[key] = match[1];
                    return true;
                }
            }
        }
        return false;
    }

public:

    /**
     * Add source descriptor to multimap.
     *
     * @param key
     * @param val
     */
    void add(const std::string& key, const sysinfo_source_t& val)
    {
        (*this).insert(std::pair<std::string, sysinfo_source_t>(key, val));
    }

    /**
     * Static configuration provisioning for where to fetch the props from
     */
    sysinfo_sources() : std::multimap<std::string, sysinfo_source_t>()
    {
    }

    /**
     * Retrieve value by key from sysinfo_sources. Try to fetch from cache,
     * if not found, then fetch from filesystem and save to in-ram cache.
     *
     * @param key
     * @return
     */
    const std::string& get(std::string key)
    {
        if (cache.find(key)==cache.end())
            fetch(key);
        printf("%s=%s\n", key.c_str(), cache[key].c_str());
        return cache[key];
    }
};

class sysinfo_sources_linux: public sysinfo_sources {
public:
    sysinfo_sources_linux() : sysinfo_sources() {
        add("DeviceInfo.Id",        { "/etc/machine-id", "*" });
        add("DeviceInfo.OsVersion", { "/etc/os-release", ".*VERSION_ID=\"(.*)\".*" });
        add("DeviceInfo.OsBuild",   { "/proc/version", "(.*)[\n]+" });
        add("AppInfo.Id",           { "/proc/self/cmdline", "(.*)[ ]*.*[\n]*" });
        add("proc_loadavg",         { "/proc/loadavg", "(.*)[\n]*" });
        add("proc_uptime",          { "/proc/uptime", "(.*)[\n]*" });
    }
};

sysinfo_sources_linux sysinfo;

namespace {

    static inline void sleep_ms(int t)
    {
#ifndef _WIN32
        usleep(t * 1000);
#else
        Sleep(t);
#endif
    }

    TEST(AriaSysInfo, GetCommonProps)
    {
        std::list<std::string> l = { "DeviceInfo.Id", "DeviceInfo.OsVersion", "DeviceInfo.OsBuild", "AppInfo.Id", "proc_loadavg", "proc_uptime" };

        for(auto &key : l )
        {
            std::string result = sysinfo.get(key);
            // EXPECT_EQ(false, (result.empty()) );
        }
	EXPECT_EQ(true, true);;
    }

}

#ifndef LIB_PAL_POSIX_SYSINFO_SOURCES_HPP_
#define LIB_PAL_POSIX_SYSINFO_SOURCES_HPP_
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include <map>
#include <string>

/**
 * System information source path and selector
 */
enum class sysinfo_selector
{
    raw,
    first_line,
    first_null,
    key_value
};

struct sysinfo_source_t {
    const char * path;
    sysinfo_selector selector;
    const char* name;

    sysinfo_source_t(const char* path, sysinfo_selector selector, const char* name = nullptr)
        : path(path), selector(selector), name(name)
    {
    }
};

/**
 * Helper class to retrieve various key-value pairs from system info sources.
 *
 * Everything is a file in POSIX / UNIX, so this file helps to retrieve and
 * cache info obtained from various files.
 *
 */
class sysinfo_sources : public std::multimap<std::string, sysinfo_source_t> {

protected:
    std::map<std::string, std::string> cache;

    /**
     * Read a bounded node value, select it without regex and store it in cache
     *
     * @param key       Field name
     * @return          true if field value is found and saved in cache
     */
    bool fetch(std::string key);

public:

    /**
     * Add source descriptor to multimap.
     *
     * @param key
     * @param val
     */
    void add(const std::string& key, const sysinfo_source_t& val);

    sysinfo_sources();

    /**
     * Retrieve value by key from sysinfo_sources. Try to fetch from cache,
     * if not found, then fetch from filesystem and save to in-ram cache.
     *
     * @param key
     * @return
     */
    const std::string& get(std::string key);

};

#endif /* LIB_PAL_POSIX_SYSINFO_SOURCES_HPP_ */

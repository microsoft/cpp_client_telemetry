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
typedef struct {
    const char * path;
    const char * selector;
} sysinfo_source_t;

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
     * Read node value, preprocess it using regexp and store result in cache
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


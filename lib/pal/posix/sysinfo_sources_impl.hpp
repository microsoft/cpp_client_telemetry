#ifndef LIB_PAL_POSIX_SYSINFO_SOURCES_IMPL_HPP_
#define LIB_PAL_POSIX_SYSINFO_SOURCES_IMPL_HPP_
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "sysinfo_sources.hpp"

class sysinfo_sources_impl: public sysinfo_sources {

public:

    sysinfo_sources_impl();
    
    /**
     * Get instance for serving all singleton calls
     */
    static sysinfo_sources_impl& GetSysInfo()
    {
        static sysinfo_sources_impl instance;
        return instance;
    }
};

#endif /* LIB_PAL_POSIX_SYSINFO_SOURCES_IMPL_HPP_ */


//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "IBackoff.hpp"
#include "Backoff_ExponentialWithJitter.hpp"
#include <sstream>

namespace MAT_NS_BEGIN {


std::unique_ptr<IBackoff> IBackoff::createFromConfig(std::string const& config)
{
    std::unique_ptr<IBackoff> result;

    std::istringstream is(config);
    // Force the classic "C" locale (decimal dot in doubles etc.)
    is.imbue(std::locale::classic());

    char kind = (char)is.get();
    if (is.get() != ',') {
        return result;
    }

    if (kind == 'E') {
        // "E,initialDelayMs,maximumDelayMs,multiplier,jitter"
        char sep[3] = {};
        int initialDelayMs, maximumDelayMs;
        double multiplier, jitter;
        is >> initialDelayMs >> sep[0] >> maximumDelayMs >> sep[1] >> multiplier >> sep[2] >> jitter;
        if (!is.fail() && is.get() == EOF && sep[0] == ',' && sep[1] == ',' && sep[2] == ',') {
            result.reset(new Backoff_ExponentialWithJitter(initialDelayMs, maximumDelayMs, multiplier, jitter));
            if (!static_cast<Backoff_ExponentialWithJitter*>(result.get())->good()) {
                result.reset();
            }
        }
    }

    return result;
}


} MAT_NS_END


//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef RECORDFLAGCONSTANTS_HPP
#define RECORDFLAGCONSTANTS_HPP

#include "ctmacros.hpp"

#include <cstdint>

namespace MAT_NS_BEGIN {

    // On-wire CS protocol record.flags bits. These are distinct from the
    // API-surface MICROSOFT_EVENTTAG_* policy flags and are remapped onto
    // record.flags by EventPropertiesDecorator. Kept in a dedicated header so
    // other components (e.g. the stats pipeline) can reference a single bit
    // definition without depending on the decorator's inline implementation.
    // Ref CS2.1+: https://osgwiki.com/wiki/CommonSchema/flags
    // (API-surface MICROSOFT_EVENTTAG_MARK_PII 0x08000000)
    static constexpr std::int64_t RECORD_FLAGS_EVENTTAG_MARK_PII = 0x00080000;
    // (API-surface MICROSOFT_EVENTTAG_HASH_PII 0x04000000)
    static constexpr std::int64_t RECORD_FLAGS_EVENTTAG_HASH_PII = 0x00100000;
    // (API-surface MICROSOFT_EVENTTAG_DROP_PII 0x02000000)
    static constexpr std::int64_t RECORD_FLAGS_EVENTTAG_DROP_PII = 0x00200000;
    static constexpr std::int64_t RECORD_FLAGS_EVENTTAG_SCRUB_IP = 0x00400000;

} MAT_NS_END

#endif // RECORDFLAGCONSTANTS_HPP

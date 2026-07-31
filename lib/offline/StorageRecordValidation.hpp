//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#ifndef STORAGERECORDVALIDATION_HPP
#define STORAGERECORDVALIDATION_HPP

#include "IOfflineStorage.hpp"

namespace MAT_NS_BEGIN {

    inline bool IsValidDiskStorageRecord(StorageRecord const& record)
    {
        return !(record.id.empty() || record.tenantToken.empty() ||
            static_cast<int>(record.latency) < 0 || record.timestamp <= 0);
    }

} MAT_NS_END

#endif

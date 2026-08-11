//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IOFFLINESTORAGEPROVIDER_HPP
#define IOFFLINESTORAGEPROVIDER_HPP

#include "IOfflineStorage.hpp"

#include <memory>

namespace MAT_NS_BEGIN
{
    class IOfflineStorageProvider
    {
    public:
        virtual ~IOfflineStorageProvider() = default;

        // Implementations may be shared by multiple handlers and must be
        // thread-safe when Initialize is called concurrently.
        virtual std::shared_ptr<IOfflineStorage> CreateDiskStorage(
            ILogManager& logManager, IRuntimeConfig& runtimeConfig) = 0;

        virtual std::shared_ptr<IOfflineStorage> CreateMemoryStorage(
            ILogManager& logManager, IRuntimeConfig& runtimeConfig) = 0;
    };
}
MAT_NS_END

#endif  // IOFFLINESTORAGEPROVIDER_HPP

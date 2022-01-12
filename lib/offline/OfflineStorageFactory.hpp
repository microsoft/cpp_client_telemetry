//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef OFFLINESTORAGEFACTORY_HPP
#define OFFLINESTORAGEFACTORY_HPP

#include "IOfflineStorage.hpp"
#include "api/IRuntimeConfig.hpp"

namespace MAT_NS_BEGIN
{
    class OfflineStorageFactory
    {
       public:
        static std::shared_ptr<IOfflineStorage> Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig);
    };
}
MAT_NS_END

#endif  // HTTPCLIENTFACTORY_HPP


// Copyright (c) Microsoft. All rights reserved.
#ifndef OFFLINESTORAGEFACTORY_HPP
#define OFFLINESTORAGEFACTORY_HPP

#ifdef HAVE_MAT_STORAGE
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

#endif // HAVE_MAT_STORAGE

#endif  // HTTPCLIENTFACTORY_HPP

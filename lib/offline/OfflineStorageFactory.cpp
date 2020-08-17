// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#include "OfflineStorageFactory.hpp"

namespace ARIASDK_NS_BEGIN
{
    std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
    {
#ifdef HAVE_MAT_STORAGE
        auto offlineStorageModule = logManager.GetLogConfiguration().GetModule(CFG_MODULE_OFFLINE_STORAGE);
        if ( nullptr != offlineStorageModule ) {
            LOG_TRACE("Creating OfflineStorage from module");
            return std::static_pointer_cast<IOfflineStorage>(offlineStorageModule);
        }
#ifdef USE_ROOM
        LOG_TRACE("Creating OfflineStorage_Room");
        return std::shared_ptr<IOfflineStorage>(new OfflineStorage_Room(logManager, runtimeConfig));
#else
        LOG_TRACE("Creating OfflineStorage_SQLite");
        return std::shared_ptr<IOfflineStorage>(new OfflineStorage_SQLite(logManager, runtimeConfig));
#endif //USE_ROOM
#else
        LOG_TRACE("MAT storage disabled");
        return nullptr;
#endif //HAVE_MAT_STORAGE
    }
}
ARIASDK_NS_END

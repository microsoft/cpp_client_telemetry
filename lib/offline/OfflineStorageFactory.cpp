// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#include "OfflineStorageFactory.hpp"

#ifdef USE_ROOM
#include "offline/OfflineStorage_Room.hpp"
#else
#include "offline/OfflineStorage_SQLite.hpp"
#endif

namespace MAT_NS_BEGIN
{
    std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
    {
#ifdef HAVE_MAT_STORAGE
        std::shared_ptr<IModule> module = logManager.GetLogConfiguration().GetModule(CFG_MODULE_OFFLINE_STORAGE);
        if ( nullptr != module ) {
            LOG_TRACE("Creating OfflineStorage from module");
            return std::static_pointer_cast<IOfflineStorage>(std::static_pointer_cast<IOfflineStorageModule>(module));
        }
#ifdef USE_ROOM
        LOG_TRACE("Creating OfflineStorage_Room");
        return std::make_shared<OfflineStorage_Room>(logManager, runtimeConfig);
#else
        LOG_TRACE("Creating OfflineStorage_SQLite");
        return std::make_shared<OfflineStorage_SQLite>(logManager, runtimeConfig);
#endif //USE_ROOM
#else
        LOG_TRACE("MAT storage disabled");
        return nullptr;
#endif //HAVE_MAT_STORAGE
    }
}
MAT_NS_END

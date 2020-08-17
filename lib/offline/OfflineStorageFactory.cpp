// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#include "OfflineStorageFactory.hpp"

namespace ARIASDK_NS_BEGIN
{
#ifdef HAVE_MAT_STORAGE
#ifdef USE_OFFLINE_STORAGE_MODULE
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage from module");
    return std::static_pointer_cast<IOfflineStorage>(logManager.GetLogConfiguration().GetModule(CFG_MODULE_OFFLINE_STORAGE));
}
#elif USE_ROOM
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage_Room");
    return std:shared_ptr<IOfflineStorage>(new OfflineStorage_Room(logManager, runtimeConfig));
}
#else
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage_SQLite");
    return std::shared_ptr<IOfflineStorage>(new OfflineStorage_SQLite(logManager, runtimeConfig));
}
#endif // USE_OFFLINE_STORAGE_MODULE
#else
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("MAT storage disabled");
    return nullptr;
}
#endif //HAVE_MAT_STORAGE
}
ARIASDK_NS_END

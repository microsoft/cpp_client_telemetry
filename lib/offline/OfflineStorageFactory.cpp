// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#ifdef HAVE_MAT_STORAGE

#include "OfflineStorageFactory.hpp"
#include "pal/PAL.hpp"
#ifdef USE_OFFLINE_STORAGE_MODULE
namespace ARIASDK_NS_BEGIN
{
    std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage from module");
    return std::static_pointer_cast<IOfflineStorage>(configuration.GetModule(CFG_MODULE_OFFLINE_STORAGE));
}
#elif USE_ROOM
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage_Room");
    return std::make_shared<IOfflineStorage>(new OfflineStorage_Room(logManager, config));
}
#else
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating HttpClient_WinRt");
    return std::make_shared<IOfflineStorage>(new OfflineStorage_SQLite(logManager, config));
}
#endif // USE_OFFLINE_STORAGE_MODULE
#else
std::shared_ptr<IOfflineStorage> OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("MAT storage disabled");
    return nullptr;
}
#endif
}
ARIASDK_NS_END
#endif  // HAVE_MAT_STORAGE

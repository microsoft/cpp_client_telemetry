// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#ifdef HAVE_MAT_STORAGE

#include "OfflineStorageFactory.hpp"
#include "pal/PAL.hpp"
#ifdef USE_OFFLINE_STORAGE_MODULE
namespace ARIASDK_NS_BEGIN
{
IOfflineStorage* OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage from module");
    auto ptr = std::static_pointer_cast<IOfflineStorage>(configuration.GetModule(CFG_MODULE_OFFLINE_STORAGE));
    return ptr.get();
 //   return std::static_pointer_cast<IOfflineStorage>(configuration.GetModule(CFG_MODULE_OFFLINE_STORAGE));
}
#elif USE_ROOM
IOfflineStorage* OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating OfflineStorage_Room");
    return new OfflineStorage_Room(logManager, runtimeConfig);
}
#else
IOfflineStorage* OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("Creating HttpClient_WinRt");
    return new OfflineStorage_SQLite(logManager, runtimeConfig);
}
#endif // USE_OFFLINE_STORAGE_MODULE
#else
IOfflineStorage* OfflineStorageFactory::Create(ILogManager& logManager, IRuntimeConfig& runtimeConfig)
{
    LOG_TRACE("MAT storage disabled");
    return nullptr;
}
}
ARIASDK_NS_END
#endif  // HAVE_MAT_STORAGE

// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include "utils/Utils.hpp"
#include "controlplane/SingleControlPlane.hpp"
#include "controlplane/DiskLocalStorage.hpp"
#include "controlplane/TenantDataSerializer.hpp"
#include "offline/FifoFileStorage.hpp"
#include <memory>

using namespace ARIASDK_NS::ControlPlane;

namespace ARIASDK_NS_BEGIN
{
    enum SupportLevels
    {
        Invalid,
        CollectorV0,
    };

    static SupportLevels ValidateAndDetermineConfigLevel(const ControlPlaneConfiguration& config)
    {
        if (config.structSize == (sizeof(size_t) + sizeof(LPCSTR)))
            return (config.cacheFilePathRoot == nullptr) ? SupportLevels::Invalid : SupportLevels::CollectorV0;

        return SupportLevels::Invalid;
    }

    /// <summary>
    /// Get an IControlPlane implementation that matches the provided config
    /// </summary>
    /// <returns>The IControlPlane object</returns>
    IControlPlane* GetCollectorV0ControlPlane(const ControlPlaneConfiguration& config)
    {
        std::string pathToCache(config.cacheFilePathRoot);
        pathToCache += "\\ACP";
        std::unique_ptr<ITenantDataSerializer> serializer = std::make_unique<TenantDataSerializer>();
        std::unique_ptr<ILocalStorageReader> reader(new DiskLocalStorage(pathToCache, serializer));
        return new SingleControlPlane(reader);
    }

    /// <summary>
    /// Get an IControlPlane implementation that matches the provided config
    /// </summary>
    /// <returns>The IControlPlane object</returns>
    IControlPlane* ControlPlaneProvider::GetControlPlane(const ControlPlaneConfiguration& config)
    {
        switch (ValidateAndDetermineConfigLevel(config))
        {
        case SupportLevels::CollectorV0:
            return GetCollectorV0ControlPlane(config);

        default:
            return nullptr;
        }
    }
} ARIASDK_NS_END

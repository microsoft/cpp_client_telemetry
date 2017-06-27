// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <fstream>
#include "LocalStorageConcurrentObservable.hpp"
#include <memory>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    /// <summary>
    /// Disk-based implementation of ILocalStorageReader. This class combines the behavior of
    /// serialization, storage, and notification (each implemented in a separate class) into
    /// a single unit for tenantized settings.
    /// </summary>
    class DiskLocalStorage : public ILocalStorageReader
    {
    private:
        LocalStorageConcurrentObservable m_observable;
        const std::string m_pathToCache;
        std::unique_ptr<ITenantDataSerializer> m_serializer;

        void GetPathToTenantDataFile(const ARIASDK_NS::GUID_t& ariaTenantId, std::string& pathToTenantDataFile) const;

    protected:
        /// <summary>
        /// Virtual to allow unit tests to be independent of file system
        /// </summary>
        virtual std::istream * OpenStream(const std::string& pathToFile) const;  // Exists for unit testing

    public:
        /// <summary>
        /// Ctor
        /// </summary>
        /// <paramName>Where the place cache files. Ownersip is NOT transferred to the new object</paramName>
        /// <paramName>serializer is the ITenantDataSerializer to use. Ownership is transferred to the new object</paramName>
        /// <returns>The available tenant data. Returns nullptr if no data exists</returns>
        DiskLocalStorage(const std::string& pathToCache, std::unique_ptr<ITenantDataSerializer> &serializer);

        /// <summary>
        /// Dtor
        /// </summary>
        virtual ~DiskLocalStorage();

        /// <summary>
        /// Retrieve all data for the specified tenant
        /// </summary>
        /// <returns>The available tenant data. Returns nullptr if no data exists</returns>
        virtual TenantDataPtr ReadTenantData(const ARIASDK_NS::GUID_t& ariaTenantId) const override;

        /// <summary>
        /// Register a handler to receive notifications if any parameters change within this store
        /// </summary>
        virtual void RegisterChangeEventHandler(ILocalStorageChangeEventHandler * handler) override;

        /// <summary>
        /// Unregister a previously regsitered handler
        /// </summary>
        virtual void UnregisterChangeEventHandler(ILocalStorageChangeEventHandler * handler) override;
    };

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane

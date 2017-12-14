// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include "DiskLocalStorage.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  { namespace ControlPlane
// *INDENT-ON*
{
    /// <summary>
    /// Ctor
    /// </summary>
    /// <paramName>Where the place cache files. Ownersip is NOT transferred to the new object</paramName>
    /// <paramName>serializer is the ITenantDataSerializer to use. Ownership is transferred to the new object</paramName>
    /// <returns>The available tenant data. Returns nullptr if no data exists</returns>
    DiskLocalStorage::DiskLocalStorage(const std::string& pathToCache, std::unique_ptr<ITenantDataSerializer> &serializer)
        : m_pathToCache(pathToCache)
    {
        m_serializer.swap(serializer);
    }

    /// <summary>
    /// Dtor
    /// </summary>
    DiskLocalStorage::~DiskLocalStorage()
    {
    }

    /// <summary>
    /// Virtual to allow unit tests to be independent of file system
    /// </summary>
    std::istream * DiskLocalStorage::OpenStream(const std::string& pathToFile) const
    {
        return new std::ifstream(pathToFile, std::ios::in);
    }

    /// <summary>
    /// Retrieve all data for the specified tenant
    /// </summary>
    /// <returns>The available tenant data. Returns nullptr if no data exists</returns>
    TenantDataPtr DiskLocalStorage::ReadTenantData(const ARIASDK_NS::GUID_t& ariaTenantId) const
    {
        std::string pathToTenantDataFile;
        TenantData * tenantDataPtr = nullptr;

        GetPathToTenantDataFile(ariaTenantId, pathToTenantDataFile);

        std::unique_ptr<std::istream> stream(OpenStream(pathToTenantDataFile));

        if (stream != nullptr)
        {
            // Get length of file, then read it into a string
            stream->seekg(0, std::ios_base::end);
            std::streampos size = stream->tellg();
            stream->seekg(0, std::ios_base::beg);

            if (size > 0)
            {
                std::string contents(static_cast<DWORD>(size), '\0');

                if (stream->read(&contents[0], size))
                {
                    tenantDataPtr = m_serializer->DeserializeTenantData(contents);
                }
            }
        }

        return tenantDataPtr;
    }


    void DiskLocalStorage::GetPathToTenantDataFile(const ARIASDK_NS::GUID_t& ariaTenantId, std::string& pathToTenantDataFile) const
    {
        pathToTenantDataFile = m_pathToCache + "\\" + ariaTenantId.to_string() + ".dat";
    }

    /// <summary>
    /// Register a handler to receive notifications if any parameters change within this store
    /// </summary>
    void DiskLocalStorage::RegisterChangeEventHandler(ILocalStorageChangeEventHandler * handler)
    {
        m_observable.Register(handler);
    }

    /// <summary>
    /// Unregister a previously regsitered handler
    /// </summary>
    void DiskLocalStorage::UnregisterChangeEventHandler(ILocalStorageChangeEventHandler * handler)
    {
        m_observable.Unregister(handler);
    }

}}}} // namespace Microsoft::Applications::Events ::ControlPlane

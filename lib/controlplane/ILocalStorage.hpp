// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <unordered_map>
#include "EventProperty.hpp"
#include "IControlPlane.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  { namespace ControlPlane
// *INDENT-ON*
{
    // Forward reference is needed for change handler declaration
    class ILocalStorageReader;

    /// <summary>
    /// Callers of the ILocalStorageReader API receive change notifications through this API
    /// </summary>
    class ILocalStorageChangeEventHandler
    {
    public:
        virtual ~ILocalStorageChangeEventHandler() {}

        /// <summary>
        /// This method is called once when the given localStorgeReader/tenantId changes
        /// </summary>
        virtual void OnChange(ILocalStorageReader& localStorageReader, const GUID_t& ariaTenantId) = 0;
    };

    /// <summary>
    /// This struct holds the data for a given localStorageReader/ariaTenantId combination
    /// </summary>
    struct TenantData
    {
        // Setttings
        std::unordered_map <std::string, std::string> m_stringMap;
        std::unordered_map <std::string, long> m_longMap;
        std::unordered_map <std::string, bool> m_boolMap;

        // "Real" data providers should set this to false!
        bool m_isDummy;

        TenantData()
        {
            m_isDummy = true;
        }
    };

    typedef TenantData * const TenantDataPtr;

    /// <summary>
    /// API to read tenant data from local storage
    /// </summary>
    class ILocalStorageReader
    {
    public:
        virtual ~ILocalStorageReader() {}

        /// <summary>
        /// Retrieve all data for the specified tenant
        /// </summary>
        /// <returns>The available tenant data. Returns nullptr if no data exists</returns>
        virtual TenantDataPtr ReadTenantData(const ARIASDK_NS::GUID_t& ariaTenantId) const = 0;

        /// <summary>
        /// Register a handler to receive notifications if any parameters change within this store
        /// </summary>
        virtual void RegisterChangeEventHandler(ILocalStorageChangeEventHandler * handler) = 0;

        /// <summary>
        /// Unregister a previously regsitered handler
        /// </summary>
        virtual void UnregisterChangeEventHandler(ILocalStorageChangeEventHandler * handler) = 0;
    };

    /// <summary>
    /// API to serialize/deserialize TenantData
    /// </summary>
    class ITenantDataSerializer
    {
    public:
        virtual ~ITenantDataSerializer() {}

        /// <summary>
        /// Convert the specified TenantDataPtr to a string
        /// </summary>
        /// <returns>The serialized form of the data</returns>
        virtual std::string SerializeTenantData(TenantDataPtr tenantData) const = 0;

        /// <summary>
        /// Convert the specfied string into a TenantDataPtr
        /// </summary>
        /// <returns>The appropriate TenantDataPtr, or an empty (non-dummy) TenantDataPtr on error. The caller owns deleting the object</returns>
        virtual TenantDataPtr DeserializeTenantData(const std::string& string) const = 0;
    };

}}}} // namespace Microsoft::Applications::Events ::ControlPlane

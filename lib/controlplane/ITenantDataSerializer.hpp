// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <unordered_map>
#include "EventProperty.hpp"
#include "IControlPlane.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
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

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane

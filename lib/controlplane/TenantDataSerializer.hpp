// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <unordered_map>
#include "ILocalStorage.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  { namespace ControlPlane
// *INDENT-ON*
{
    /// <summary>
    /// API to serialize/deserialize TenantData
    /// </summary>
    class TenantDataSerializer : public ITenantDataSerializer
    {
    public:
        /// <summary>
        /// Convert the specified TenantDataPtr to a string
        /// </summary>
        /// <returns>The serialized form of the data</returns>
        virtual std::string SerializeTenantData(TenantDataPtr tenantData) const override;

        /// <summary>
        /// Convert the specfied string into a TenantDataPtr
        /// </summary>
        /// <returns>The appropriate TenantDataPtr, or an empty (non-dummy) TenantDataPtr on error. The caller owns deleting the object</returns>
        virtual TenantDataPtr DeserializeTenantData(const std::string& stringToConvert) const override;

    private:
        static LPCSTR BOOL_MAP_NAME;
        static LPCSTR LONG_MAP_NAME;
        static LPCSTR STRING_MAP_NAME;
    };

}}}} // namespace Microsoft::Applications::Events ::ControlPlane

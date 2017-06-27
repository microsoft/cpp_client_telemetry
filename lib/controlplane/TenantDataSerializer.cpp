// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <json.hpp>
#include "TenantDataSerializer.hpp"

using namespace nlohmann;

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    LPCSTR TenantDataSerializer::BOOL_MAP_NAME = "BoolMap";
    LPCSTR TenantDataSerializer::LONG_MAP_NAME = "LongMap";
    LPCSTR TenantDataSerializer::STRING_MAP_NAME = "StringMap";

    /// <summary>
    /// Convert the specified TenantDataPtr to a string
    /// </summary>
    /// <returns>The serialized form of the data</returns>
    std::string TenantDataSerializer::SerializeTenantData(TenantDataPtr tenantData) const
    {
        if (tenantData == nullptr)
            return std::string();

        // Serializer returns a string saying "NULL" if we call it without setting any fields,
        // so special-case the output if no settings were found.
        bool settingsExist = false;
        json serializer;

        for (auto pair : tenantData->m_boolMap)
        {
            serializer[BOOL_MAP_NAME][pair.first] = pair.second;
            settingsExist = true;
        }
        for (auto pair : tenantData->m_longMap)
        {
            serializer[LONG_MAP_NAME][pair.first] = pair.second;
            settingsExist = true;
        }
        for (auto pair : tenantData->m_stringMap)
        {
            serializer[STRING_MAP_NAME][pair.first] = pair.second;
            settingsExist = true;
        }

        return settingsExist ? serializer.dump() : std::string();
    }

    /// <summary>
    /// Convert the specfied string into a TenantDataPtr
    /// </summary>
    /// <returns>The appropriate TenantDataPtr, or an empty (non-dummy) TenantDataPtr on error. The caller owns deleting the object</returns>
    TenantDataPtr TenantDataSerializer::DeserializeTenantData(const std::string& stringToConvert) const
    {
        try
        {
            json parsedData = json::parse(stringToConvert.c_str());

            auto boolMap = parsedData.find(BOOL_MAP_NAME);
            auto longMap = parsedData.find(LONG_MAP_NAME);
            auto stringMap = parsedData.find(STRING_MAP_NAME);

            TenantData * tenantDataPtr = new TenantData();

            if (boolMap != parsedData.end())
            {
                auto map = boolMap.value();
                for (auto it = map.begin(); it != map.end(); ++it)
                {
                    tenantDataPtr->m_boolMap[it.key()] = it.value();
                }
            }

            if (longMap != parsedData.end())
            {
                auto map = longMap.value();
                for (auto it = map.begin(); it != map.end(); ++it)
                {
                    tenantDataPtr->m_longMap[it.key()] = it.value();
                }
            }

            if (stringMap != parsedData.end())
            {
                auto map = stringMap.value();
                for (auto it = map.begin(); it != map.end(); ++it)
                {
                    tenantDataPtr->m_stringMap[it.key()] = it.value().get<std::string>();
                }
            }

            tenantDataPtr->m_isDummy = false;
            return tenantDataPtr;
        }
        catch (...)
        {
            return nullptr;
        }
    }

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane

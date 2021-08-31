//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ContextFieldsProvider.hpp"
#include "LogSessionData.hpp"
#include "pal/PAL.hpp"
#include "utils/StringUtils.hpp"

namespace MAT_NS_BEGIN
{

    ContextFieldsProvider::ContextFieldsProvider()
        : ContextFieldsProvider(nullptr)
    {
    }

    ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider* parent)
        : m_parent(parent)
    {
        if (!m_parent)
        {
            PAL::registerSemanticContext(this);
        }
    }

    ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider const& copy)
    {
        m_parent = copy.m_parent;
        m_commonContextFields = copy.m_commonContextFields;
        m_customContextFields = copy.m_customContextFields;
        m_commonContextEventToConfigIds = copy.m_commonContextEventToConfigIds;
        m_ticketsMap = copy.m_ticketsMap;
    }

    ContextFieldsProvider& ContextFieldsProvider::operator=(ContextFieldsProvider const& copy)
    {
        m_parent = copy.m_parent;
        m_commonContextFields = copy.m_commonContextFields;
        m_customContextFields = copy.m_customContextFields;
        m_commonContextEventToConfigIds = copy.m_commonContextEventToConfigIds;
        m_ticketsMap = copy.m_ticketsMap;
        return *this;
    }

    void ContextFieldsProvider::writeToRecord(::CsProtocol::Record& record, bool commonOnly)
    {
        // Append parent scope context variables if not detached from parent
        if (m_parent)
        {
            m_parent->writeToRecord(record);
        }

        if (record.data.size() == 0)
        {
            ::CsProtocol::Data data;
            record.data.push_back(data);
        }
        if (record.extApp.size() == 0)
        {
            ::CsProtocol::App app;
            record.extApp.push_back(app);
        }

        if (record.extDevice.size() == 0)
        {
            ::CsProtocol::Device device;
            record.extDevice.push_back(device);
        }

        if (record.extOs.size() == 0)
        {
            ::CsProtocol::Os os;
            record.extOs.push_back(os);
        }

        if (record.extUser.size() == 0)
        {
            ::CsProtocol::User user;
            record.extUser.push_back(user);
        }

        if (record.extLoc.size() == 0)
        {
            ::CsProtocol::Loc loc;
            record.extLoc.push_back(loc);
        }

        if (record.extNet.size() == 0)
        {
            ::CsProtocol::Net net;
            record.extNet.push_back(net);
        }

        if (record.extProtocol.size() == 0)
        {
            ::CsProtocol::Protocol proto;
            record.extProtocol.push_back(proto);
        }

        if (record.extM365a.size() == 0)
        {
            ::CsProtocol::M365a m365a;
            record.extM365a.push_back(m365a);
        }

        std::map<std::string, ::CsProtocol::Value>& ext = record.data[0].properties;
        {
            LOCKGUARD(m_lock);

            std::string value = m_commonContextFields[COMMONFIELDS_APP_EXPERIMENTIDS].as_string;
            if (!value.empty())
            {// for ECS set event specific config ids
                std::string eventName = record.name;
                if (!eventName.empty())
                {
                    const auto& iter = m_commonContextEventToConfigIds.find(eventName);
                    if (iter != m_commonContextEventToConfigIds.end())
                    {
                        value = iter->second;
                    }
                }

                record.extApp[0].expId = value;
            }

            if (!m_commonContextFields.empty())
            {
                if (m_commonContextFields.find(SESSION_IMPRESSION_ID) != m_commonContextFields.end())
                {
                    CsProtocol::Value temp;
                    EventProperty prop = m_commonContextFields[SESSION_IMPRESSION_ID];
                    temp.stringValue = prop.as_string;

                    ext[SESSION_IMPRESSION_ID] = temp;
                }

                if (m_commonContextFields.find(COMMONFIELDS_APP_EXPERIMENTETAG) != m_commonContextFields.end())
                {
                    CsProtocol::Value temp;
                    EventProperty prop = m_commonContextFields[COMMONFIELDS_APP_EXPERIMENTETAG];
                    temp.stringValue = prop.as_string;

                    ext[COMMONFIELDS_APP_EXPERIMENTETAG] = temp;
                }

                auto iter = m_commonContextFields.find(COMMONFIELDS_APP_ID);
                bool hasAppId = (iter != m_commonContextFields.end());
                if (hasAppId)
                {
                    record.extApp[0].id = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_APP_ENV);
                bool hasAppEnv = (iter != m_commonContextFields.end());
                if (hasAppEnv)
                {
                    record.extApp[0].env = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_APP_NAME);
                if (iter != m_commonContextFields.end())
                {
                    record.extApp[0].name = iter->second.as_string;
                }
                else if (hasAppId)
                {
                    // Backwards-compat: legacy Aria exporter maps CS3.0 ext.app.name to AppInfo.Id
                    // TODO:
                    // - consider resolving that protocol "wrinkle" backend-side
                    // - consider parsing ext.app.id if it contains app hash!name:ver information
                    record.extApp[0].name = record.extApp[0].id;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_APP_VERSION);
                if (iter != m_commonContextFields.end())
                {
                    record.extApp[0].ver = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_APP_LANGUAGE);
                if (iter != m_commonContextFields.end())
                {
                    record.extApp[0].locale = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_DEVICE_ID);
                if (iter != m_commonContextFields.end())
                {
                    // Use "c:" prefix
                    std::string temp("c:");
                    const char *deviceId = iter->second.as_string;
                    if (deviceId != nullptr)
                    {
                        size_t len = strlen(deviceId);
                        if (len >= 2 && deviceId[1] == ':' && (
                            deviceId[0] == 'c' || // c: Custom identifier
                            deviceId[0] == 'r' || // r: Randomized identifier
                            deviceId[0] == 'u' || // u: Mac OS X UUID
                            deviceId[0] == 'a' || // a: Android ID
                            deviceId[0] == 's' || // s: SQM ID
                            deviceId[0] == 'x' || // x: XBox One hardware ID
                            deviceId[0] == 'i'))  // i: iOS ID
                        {
                            // Remove "c:" prefix
                            temp = "";
                        }
                        // Strip curly braces from GUID while populating localId.
                        // Otherwise 1DS collector would not strip the prefix.
                        if ((deviceId[0] == '{') && (deviceId[len - 1] == '}'))
                        {
                            temp.append(deviceId + 1, len - 2);
                        }
                        else
                        {
                            temp.append(deviceId);
                        }
                    }
                    record.extDevice[0].localId = temp;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_DEVICE_ORGID);
                if (iter != m_commonContextFields.end())
                {
                    record.extDevice[0].orgId = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_DEVICE_MAKE);
                if (iter != m_commonContextFields.end())
                {
                    record.extProtocol[0].devMake = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_DEVICE_MODEL);
                if (iter != m_commonContextFields.end())
                {
                    record.extProtocol[0].devModel = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_DEVICE_CLASS);
                if (iter != m_commonContextFields.end())
                {
                    record.extDevice[0].deviceClass = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_COMMERCIAL_ID);
                if (iter != m_commonContextFields.end())
                {
                    record.extM365a[0].enrolledTenantId = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_OS_NAME);
                if (iter != m_commonContextFields.end())
                {
                    record.extOs[0].name = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_OS_BUILD);
                if (iter != m_commonContextFields.end())
                {
                    //EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_OS_VERSION];
                    record.extOs[0].ver = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_USER_ID);
                if (iter != m_commonContextFields.end())
                {
                    record.extUser[0].localId = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_USER_LANGUAGE);
                if (iter != m_commonContextFields.end())
                {
                    record.extUser[0].locale = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_USER_TIMEZONE);
                if (iter != m_commonContextFields.end())
                {
                    record.extLoc[0].timezone = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_NETWORK_COST);
                if (iter != m_commonContextFields.end())
                {
                    record.extNet[0].cost = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_NETWORK_PROVIDER);
                if (iter != m_commonContextFields.end())
                {
                    record.extNet[0].provider = iter->second.as_string;
                }

                iter = m_commonContextFields.find(COMMONFIELDS_NETWORK_TYPE);
                if (iter != m_commonContextFields.end())
                {
                    record.extNet[0].type = iter->second.as_string;
                }
            }

            if (m_ticketsMap.size() > 0)
            {
                std::vector<std::string> tickets;
                for (auto const& field : m_ticketsMap)
                {
                    tickets.push_back(field.second);
                }
                CsProtocol::Protocol temp;
                temp.ticketKeys.push_back(tickets);
                record.extProtocol.push_back(temp);
            }

            if (!commonOnly)
            {
                for (auto const& field : m_customContextFields)
                {
                    if (field.second.piiKind != PiiKind_None)
                    {
                        CsProtocol::PII pii;
                        pii.Kind = static_cast<CsProtocol::PIIKind>(field.second.piiKind);
                        CsProtocol::Value temp;
                        CsProtocol::Attributes attrib;
                        attrib.pii.push_back(pii);


                        temp.attributes.push_back(attrib);

                        temp.stringValue = field.second.to_string();
                        record.data[0].properties[field.first] = temp;
                    }
                    else
                    {
                        std::vector<uint8_t> guid;
                        uint8_t guid_bytes[16] = { 0 };

                        switch (field.second.type)
                        {
                        case EventProperty::TYPE_STRING:
                        {
                            CsProtocol::Value temp;
                            temp.stringValue = field.second.to_string();
                            record.data[0].properties[field.first] = temp;
                            break;
                        }
                        case EventProperty::TYPE_INT64:
                        {
                            CsProtocol::Value temp;
                            temp.type = ::CsProtocol::ValueKind::ValueInt64;
                            temp.longValue = field.second.as_int64;
                            record.data[0].properties[field.first] = temp;
                            break;
                        }
                        case EventProperty::TYPE_DOUBLE:
                        {
                            CsProtocol::Value temp;
                            temp.type = ::CsProtocol::ValueKind::ValueDouble;
                            temp.doubleValue = field.second.as_double;
                            record.data[0].properties[field.first] = temp;
                            break;
                        }
                        case EventProperty::TYPE_TIME:
                        {
                            CsProtocol::Value temp;
                            temp.type = ::CsProtocol::ValueKind::ValueDateTime;
                            temp.longValue = field.second.as_time_ticks.ticks;
                            record.data[0].properties[field.first] = temp;
                            break;
                        }
                        case EventProperty::TYPE_BOOLEAN:
                        {
                            CsProtocol::Value temp;
                            temp.type = ::CsProtocol::ValueKind::ValueBool;
                            temp.longValue = field.second.as_bool;
                            record.data[0].properties[field.first] = temp;
                            break;
                        }
                        case EventProperty::TYPE_GUID:
                        {
                            GUID_t temp = field.second.as_guid;
                            temp.to_bytes(guid_bytes);
                            guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));

                            CsProtocol::Value tempValue;
                            tempValue.type = ::CsProtocol::ValueKind::ValueGuid;
                            tempValue.guidValue.push_back(guid);
                            record.data[0].properties[field.first] = tempValue;
                            break;
                        }
                        default:
                        {
                            // Convert all unknown types to string
                            CsProtocol::Value temp;
                            temp.stringValue = field.second.to_string();
                            record.data[0].properties[field.first] = temp;
                        }
                        }
                    }
                }
            }
            LOG_TRACE("Record=%p decorated with SemanticContext=%p", &record, this);
        }
    }

    void ContextFieldsProvider::ClearExperimentIds()
    {
        // Clear the common ExperimentIds
        SetCommonField(COMMONFIELDS_APP_EXPERIMENTIDS, "");

        // Clear the map of all ExperimentsIds (that's associated with event)
        m_commonContextEventToConfigIds.clear();
    }

    void ContextFieldsProvider::SetEventExperimentIds(std::string const& eventName, std::string const& experimentIds)
    {
        if (eventName.empty())
        {
            return;
        }

        std::string eventNameNormalized = toLower(eventName);
        if (!experimentIds.empty())
        {
            m_commonContextEventToConfigIds[eventNameNormalized] = experimentIds;
        }
        else
        {
            m_commonContextEventToConfigIds.erase(eventNameNormalized);
        }
    }

    void ContextFieldsProvider::SetCommonField(const std::string& name, const EventProperty& value)
    {
        LOCKGUARD(m_lock);
        m_commonContextFields[name] = value;
    }

    void ContextFieldsProvider::SetCustomField(const std::string& name, const EventProperty& value)
    {
        LOCKGUARD(m_lock);
        m_customContextFields[name] = value;
    }

    void ContextFieldsProvider::SetTicket(TicketType type, const std::string& ticketValue)
    {
        LOCKGUARD(m_lock);
        if (!ticketValue.empty())
        {
            m_ticketsMap[type] = ticketValue;
        }
    }

    void ContextFieldsProvider::SetParentContext(ContextFieldsProvider* parent)
    {
        m_parent = parent;
    }

    std::map<std::string, EventProperty>& ContextFieldsProvider::GetCommonFields()
    {
        return m_commonContextFields;
    }

    std::map<std::string, EventProperty>& ContextFieldsProvider::GetCustomFields()
    {
        return m_customContextFields;
    }

} MAT_NS_END


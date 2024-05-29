//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ContextFieldsProvider.hpp"
#include "LogSessionData.hpp"
#include "pal/PAL.hpp"
#include "utils/StringUtils.hpp"

#include <unordered_map>

namespace MAT_NS_BEGIN
{
    // clang-format off
    /**
     * This map allows to remap from canonical Common Schema JSON notation to "CommonFields"
     * (ex. AppInfo.*) notation historically used by Aria v1/v2 and 1DS v3 SDKs. Field name
     * reshaping is performed as follows:
     *
     *  CS3.0/4.0 JSON notation -> Common Alias -> :CsProtocol::Record object -> CS on wire
     *
     *  Common Alias (no reshaping)             -> :CsProtocol::Record object -> CS on wire
     *
     * Lookup for the data transform is a hashtable-based. Performed only in case if
     * customer-supplied Common Context property starts with "ext.": check 4 bytes match,
     * then perform the hash map lookup:
     * - If there is a match, promote to corresponding COMMONFIELDS_* name.
     * - If there's no match, keep as is with its original context field name.
     * This logic allows to respect both - "canonical" names and "legacy" entity names.
     *
     * Why do we have to support both naming conventions? It's an organizational choice whether
     * to rely on 'legacy' AppInfo.*, EventInfo.*, naming (Aria-Kusto) or migrate to more modern,
     * standard Common Schema notation (standalone Kusto). Organizational choice depends on
     * final data storage and consumption model. From a developer perspective - the solution is
     * to allow a developer to use the naming convention as they would use to query a dataset
     * in Kusto or ADLS Gen2. Having 1-1 mapping between instrumentation properties and storage
     * allows to resolve ambiguities and avoid confusion.
     *
     * For any extension that is not supported by the map below, a developer could implement
     * their own custom IDecorator - to stamp a common field property at decorator-level.
     */
    static const std::unordered_map<std::string, std::string> kCommonSchemaToCommonFieldsMap=
        {
            // ext.app extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/app.html
            {"ext.app.id",              COMMONFIELDS_APP_ID},
            {"ext.app.ver",             COMMONFIELDS_APP_VERSION},
            {"ext.app.name",            COMMONFIELDS_APP_NAME},
            {"ext.app.locale",          COMMONFIELDS_APP_LANGUAGE},
            // {"ext.app.asId",         NOT_SUPPORTED},
            // {"ext.app.sesId",        NOT_SUPPORTED},
            // {"ext.app.userId",       NOT_SUPPORTED}, // use "ext.user.*id" instead
            {"ext.app.expId",           COMMONFIELDS_APP_EXPERIMENTIDS},
            {"ext.app.env",             COMMONFIELDS_APP_ENV},
            // ext.device extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/device.html
            // {"ext.device.id",        NOT_SUPPORTED}, // Device g: ID can only be populated via MSAL ticket claim.
            {"ext.device.deviceClass",  COMMONFIELDS_DEVICE_CLASS},
            {"ext.device.make",         COMMONFIELDS_DEVICE_MAKE},
            {"ext.device.model",        COMMONFIELDS_DEVICE_MODEL},
            {"ext.device.localId",      COMMONFIELDS_DEVICE_ID},
            // {"ext.device.auth*",     NOT_SUPPORTED}, // Use IDecorator
            // {"ext.device.org*",      NOT_SUPPORTED}, // Use IDecorator
            // ext.net extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/net.html
            {"ext.net.provider",        COMMONFIELDS_NETWORK_PROVIDER},
            {"ext.net.cost",            COMMONFIELDS_NETWORK_COST},
            {"ext.net.type",            COMMONFIELDS_NETWORK_TYPE},
            // ext.os extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/os.html
            {"ext.os.name",             COMMONFIELDS_OS_NAME},

            // Special case for CS3.0 vs CS4.0:
            // - ext.os.ver             - exists in both - CS3.0 and CS4.0
            // - ext.os.build           - exists only in CS4.0
#if defined(HAVE_CS4) || defined(HAVE_CS4_FULL)
            {"ext.os.ver",              COMMONFIELDS_OS_VERSION},
            {"ext.os.build",            COMMONFIELDS_OS_BUILD},
#else
            // For some historical reason, the code treated
            // COMMONFIELDS_OS_BUILD as an alias for ext.os.ver.
            // Keep it that way, so we don't break the contract
            // for existing apps. COMMONFIELDS_OS_BUILD lands
            // on extOs[0].ver anyways. Thus, we keep consistency
            // for devs that use Common Schema notation.
            {"ext.os.ver",              COMMONFIELDS_OS_BUILD},
#endif
            //{"ext.os.locale",         NOT_SUPPORTED}, // Use IDecorator
            //{"ext.os.bootId",         NOT_SUPPORTED}, // Use IDecorator
            //{"ext.os.expId",          NOT_SUPPORTED}, // Use IDecorator
            // ext.user extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/user.html
            // {"ext.user.id",          NOT_SUPPORTED}, // User g: ID can only be populated via MSAL ticket claim.
            {"ext.user.localId",        COMMONFIELDS_USER_ID},
            // {"ext.user.authId",      NOT_SUPPORTED}, // Use IDecorator
            {"ext.user.locale",         COMMONFIELDS_USER_LANGUAGE},
            // ext.loc extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/loc.html
            // {"ext.loc.id",           NOT_SUPPORTED}, // Use IDecorator
            // {"ext.loc.country",      NOT_SUPPORTED}, // Use IDecorator
            {"ext.loc.tz",              COMMONFIELDS_USER_TIMEZONE},
            {"ext.loc.timezone",        COMMONFIELDS_USER_TIMEZONE}, // alias of 'tz'
            // ext.m365 extension:
            // https://1dsdocs.azurewebsites.net/schema/PartA/m365a.html
            {"ext.m365a.enrolledTenantId", COMMONFIELDS_COMMERCIAL_ID }
        };
    // clang-format on

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

#if defined(HAVE_CS4) || defined(HAVE_CS4_FULL)
                // Straightforward implementation for CS4.0+. No quirks.
                // - OS_VERSION maps to ext.os.ver field.
                iter = m_commonContextFields.find(COMMONFIELDS_OS_VERSION);
                if (iter != m_commonContextFields.end())
                {
                    record.extOs[0].ver = iter->second.as_string;
                }
                // - OS_BUILD maps to ext.os.build field.
                iter = m_commonContextFields.find(COMMONFIELDS_OS_BUILD);
                if (iter != m_commonContextFields.end())
                {
                    record.extOs[0].build = iter->second.as_string;
                }
#else
                // This is a historical quirk due to difference between CS3.0 and CS4.0
                // `ext.os.ver` exists in both schemas; but `ext.os.build` only in CS4.0
                // However, it appears like the preference in this code has been to use
                // the newer Aria-style alias. Fixing this could break existing apps.
                // Thus, we keep the legacy behavior untouched.
                iter = m_commonContextFields.find(COMMONFIELDS_OS_BUILD);
                if (iter != m_commonContextFields.end())
                {
                    //EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_OS_VERSION];
                    record.extOs[0].ver = iter->second.as_string;
                }
#endif

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
        // Any common field that starts with "ext." prefix and exists in kCommonSchemaToCommonFieldsMap
        // is considered to be a Part A extension property. Code below allows to remap from JSON CS
        // notation to CommonFields, e.g. (AppInfo.*, DeviceInfo.*, etc.) notation.
        if (name.rfind("ext.", 0)==0)
        {
            const auto it = kCommonSchemaToCommonFieldsMap.find(name);
            if (it != kCommonSchemaToCommonFieldsMap.end())
            {
                // Rename the key from Common Schema dotted notation to COMMONFIELDS_* alias
                m_commonContextFields[it->second] = value;
                return;
            }
        }
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

    void ContextFieldsProvider::ClearParentContext()
    {
        // This method allows to disassociate from parent LogManager context due to isolation
        // reasons. For example, when Guest attaches to Host LogManager, it could be configured
        // to avoid capturing common context properties. Logger context on creation by default
        // acquires the properties populated by its LogManager. Guest Logger context should be
        // wiped clean if Guest scope has been set to none ("-").
        m_parent = nullptr;
        m_commonContextFields.clear();
        m_customContextFields.clear();
        m_commonContextEventToConfigIds.clear();
        PAL::registerSemanticContext(this);
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


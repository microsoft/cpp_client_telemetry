// Copyright (c) Microsoft. All rights reserved.

#include "ContextFieldsProvider.hpp"
#include "LogSessionData.hpp"
#include "Config.hpp"
#include "utils/Utils.hpp"
#include "CommonLogManagerInternal.hpp"

namespace ARIASDK_NS_BEGIN {


ContextFieldsProvider::ContextFieldsProvider()
    : m_lockP(new std::mutex())
    , m_parent( NULL)
    , m_commonContextFieldsP(new std::map<std::string, EventProperty>())
    , m_customContextFieldsP(new std::map<std::string, EventProperty>())
    , m_commonContextEventToConfigIdsP(new std::map<std::string, std::string>())
    , m_CommonFieldsAppExperimentIdsP( new std::string())
    , m_ticketsMapP(new std::map<TicketType, std::string>())
{

}
ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider* parent)
  : m_lockP(new std::mutex())
   ,m_parent(parent)
   ,m_commonContextFieldsP( new std::map<std::string, EventProperty>())
   ,m_customContextFieldsP( new std::map<std::string, EventProperty>())
   ,m_commonContextEventToConfigIdsP(new std::map<std::string, std::string>())
   ,m_CommonFieldsAppExperimentIdsP(new std::string())
   ,m_ticketsMapP(new std::map<TicketType, std::string>())
{
    if (!m_parent) 
    {
        PAL::registerSemanticContext(this);        
        LogSessionData* sessionData = CommonLogManagerInternal::GetLogSessionData();
        if (sessionData)
        {
            setCommonField("act_session_id", CommonLogManagerInternal::GetLogSessionData()->getSessionSDKUid());
        }
        else
        {
            setCommonField("act_session_id", PAL::generateUuidString());
        }
    }
}

ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider const& copy)
{
    m_lockP = new std::mutex();
    m_commonContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_commonContextFieldsP);
    m_customContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_customContextFieldsP);
    m_commonContextEventToConfigIdsP =  new std::map<std::string, std::string>(*copy.m_commonContextEventToConfigIdsP);
    m_CommonFieldsAppExperimentIdsP = new std::string(*copy.m_CommonFieldsAppExperimentIdsP);
    m_ticketsMapP = new std::map<TicketType, std::string>(*copy.m_ticketsMapP);
}

ContextFieldsProvider& ContextFieldsProvider::operator=(ContextFieldsProvider const& copy)
{
    m_lockP = new std::mutex();
    m_commonContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_commonContextFieldsP);
    m_customContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_customContextFieldsP);
    m_commonContextEventToConfigIdsP = new std::map<std::string, std::string>(*copy.m_commonContextEventToConfigIdsP);
    m_CommonFieldsAppExperimentIdsP = new std::string(*copy.m_CommonFieldsAppExperimentIdsP);
    m_ticketsMapP = new std::map<TicketType, std::string>(*copy.m_ticketsMapP);
    return *this;
}


ContextFieldsProvider::~ContextFieldsProvider()
{
    if (!m_parent) {
        PAL::unregisterSemanticContext(this);
    }
    if (m_lockP) delete m_lockP;
    if (m_commonContextFieldsP) delete m_commonContextFieldsP;
    if (m_customContextFieldsP) delete m_customContextFieldsP;
    if (m_commonContextEventToConfigIdsP) delete m_commonContextEventToConfigIdsP;
    if (m_CommonFieldsAppExperimentIdsP) delete m_CommonFieldsAppExperimentIdsP;
    if (m_ticketsMapP) delete m_ticketsMapP;
}

void ContextFieldsProvider::setCommonField(std::string const& name, EventProperty value)
{
    std::lock_guard<std::mutex> lock(*m_lockP);

    (*m_commonContextFieldsP)[name] = value;
}

void ContextFieldsProvider::setCustomField(std::string const& name, EventProperty value)
{
    std::lock_guard<std::mutex> lock(*m_lockP);

    (*m_customContextFieldsP)[name] = value;
}

void ContextFieldsProvider::writeToRecord(::AriaProtocol::CsEvent& record) const
{
    if (m_parent)
    {
        m_parent->writeToRecord(record);
    }

    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }
    if (record.extApp.size() == 0)
    {
        ::AriaProtocol::App app;
        record.extApp.push_back(app);
    }

    if (record.extDevice.size() == 0)
    {
        ::AriaProtocol::Device device;
        record.extDevice.push_back(device);
    }

    if (record.extOs.size() == 0)
    {
        ::AriaProtocol::Os os;
        record.extOs.push_back(os);
    }

    if (record.extUser.size() == 0)
    {
        ::AriaProtocol::User user;
        record.extUser.push_back(user);
    }

    if (record.extLoc.size() == 0)
    {
        ::AriaProtocol::Loc loc;
        record.extLoc.push_back(loc);
    }

    if (record.extNet.size() == 0)
    {
        ::AriaProtocol::Net net;
        record.extNet.push_back(net);
    }
    
    std::map<std::string, ::AriaProtocol::Value>& ext = record.data[0].properties;
    {
        std::lock_guard<std::mutex> lock(*m_lockP);

        if (!m_CommonFieldsAppExperimentIdsP->empty())
        {// for ECS set event specific config ids
            std::string value = *m_CommonFieldsAppExperimentIdsP;
            std::string eventName = record.name;
            if (!eventName.empty())
            {
                std::map<std::string, std::string>::const_iterator iter = m_commonContextEventToConfigIdsP->find(eventName);
                if (iter != m_commonContextEventToConfigIdsP->end())
                {
                    value = iter->second;
                }
            }
            
            record.extApp[0].expId = value;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID) != m_commonContextFieldsP->end())
        {
            AriaProtocol::Value temp;
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID];
            temp.stringValue = prop.as_string;

            ext[COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID] = temp;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_APP_EXPERIMENTETAG) != m_commonContextFieldsP->end())
        {
            AriaProtocol::Value temp;
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_APP_EXPERIMENTETAG];
            temp.stringValue = prop.as_string;

            ext[COMMONFIELDS_APP_EXPERIMENTETAG] = temp;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_APP_ID) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_APP_ID];
            record.extApp[0].id = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_APP_VERSION) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_APP_VERSION];
            record.extApp[0].ver = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_APP_LANGUAGE) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_APP_LANGUAGE];
            record.extApp[0].locale = prop.as_string;
        }       

        if (m_commonContextFieldsP->find(COMMONFIELDS_DEVICE_ID) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_DEVICE_ID];
            record.extDevice[0].localId = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_DEVICE_MAKE) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_DEVICE_MAKE];
            record.extDevice[0].make = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_DEVICE_MODEL) != m_commonContextFieldsP->end()) 
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_DEVICE_MODEL];
            record.extDevice[0].model = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_DEVICE_CLASS) != m_commonContextFieldsP->end()) 
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_DEVICE_CLASS];
            record.extDevice[0].deviceClass = prop.as_string;
        }
        
        if (m_commonContextFieldsP->find(COMMONFIELDS_OS_NAME) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_OS_NAME];
            record.extOs[0].name = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_OS_BUILD) != m_commonContextFieldsP->end())
        {
            //EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_OS_VERSION];
            EventProperty prop1 = (*m_commonContextFieldsP)[COMMONFIELDS_OS_BUILD];// build has version in it
            record.extOs[0].ver = prop1.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_USER_ID) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_USER_ID];
            record.extUser[0].localId = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_USER_LANGUAGE) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_USER_LANGUAGE];
            record.extUser[0].locale = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_USER_TIMEZONE) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_USER_TIMEZONE];
            record.extLoc[0].timezone = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_USER_MSAID) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_USER_MSAID];
            record.extDevice[0].authSecId = prop.as_string;
        }
        
        if (m_commonContextFieldsP->find(COMMONFIELDS_NETWORK_COST) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_NETWORK_COST];
            record.extNet[0].cost = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_NETWORK_PROVIDER) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_NETWORK_PROVIDER];
            record.extNet[0].provider = prop.as_string;
        }

        if (m_commonContextFieldsP->find(COMMONFIELDS_NETWORK_TYPE) != m_commonContextFieldsP->end())
        {
            EventProperty prop = (*m_commonContextFieldsP)[COMMONFIELDS_NETWORK_TYPE];
            record.extNet[0].type = prop.as_string;
        }

        if (m_ticketsMapP->size() > 0)
        {
            std::vector<std::string> tickets;
            for (auto const& field : (*m_ticketsMapP))
            {
                tickets.push_back(field.second);
            }
            AriaProtocol::Protocol temp;
            temp.ticketKeys.push_back(tickets);
            record.extProtocol.push_back(temp);
        }

        for (auto const& field : (*m_customContextFieldsP))
        {
            if (field.second.piiKind != PiiKind_None)
            {
                AriaProtocol::PII pii;
                pii.Kind = static_cast<AriaProtocol::PIIKind>(field.second.piiKind);
                AriaProtocol::Value temp;
                AriaProtocol::Attributes attrib;
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
                        AriaProtocol::Value temp;
                        temp.stringValue = field.second.to_string();
                        record.data[0].properties[field.first] = temp;
                        break;
                    }
                    case EventProperty::TYPE_INT64:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueInt64;
                        temp.longValue = field.second.as_int64;
                        record.data[0].properties[field.first] = temp;
                        break;
                    }
                    case EventProperty::TYPE_DOUBLE:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueDouble;
                        temp.doubleValue = field.second.as_double;
                        record.data[0].properties[field.first] = temp;
                        break;
                    }
                    case EventProperty::TYPE_TIME:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueDateTime;
                        temp.longValue = field.second.as_time_ticks.ticks;
                        record.data[0].properties[field.first] = temp;
                        break;
                    }
                    case EventProperty::TYPE_BOOLEAN:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueBool;
                        temp.longValue = field.second.as_bool;
                        record.data[0].properties[field.first] = temp;
                        break;
                    }
                    case EventProperty::TYPE_GUID:
                    {
                        GUID_t temp = field.second.as_guid;
                        temp.to_bytes(guid_bytes);
                        guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));

                        AriaProtocol::Value tempValue;
                        tempValue.type = ::AriaProtocol::ValueKind::ValueGuid;
                        tempValue.guidValue.push_back(guid);
                        record.data[0].properties[field.first] = tempValue;
                        break;
                    }
                    default:
                    {
                        // Convert all unknown types to string
                        AriaProtocol::Value temp;
                        temp.stringValue = field.second.to_string();
                        record.data[0].properties[field.first] = temp;
                    }
                }
            }
        }        
    }
}

//---

// Reference: https://aria.microsoft.com/developer/do-more/working-with-data/common-properties


void ContextFieldsProvider::SetAppExperimentETag(std::string const& appExperimentETag)
{
    setCommonField(COMMONFIELDS_APP_EXPERIMENTETAG, appExperimentETag);
    _ClearExperimentIds();
}

void ContextFieldsProvider::_ClearExperimentIds()
{
    // Clear the common ExperimentIds
    m_CommonFieldsAppExperimentIdsP->clear();
    // Clear the map of all ExperimentsIds (that's associated with event)
    m_commonContextEventToConfigIdsP->clear();
}

void ContextFieldsProvider::SetAppExperimentIds(std::string const& appExperimentIds)
{
    *m_CommonFieldsAppExperimentIdsP = appExperimentIds;
}

void ContextFieldsProvider::SetAppExperimentImpressionId(std::string const& appExperimentImpressionId)
{
    setCommonField(COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID, appExperimentImpressionId);
}

void ContextFieldsProvider::SetEventExperimentIds(std::string const& eventName, std::string const& experimentIds)
{
    std::string eventNameNormalized = toLower(eventName);
    if (!eventName.empty())
    {	
        if (!experimentIds.empty())
        {
            if (m_commonContextEventToConfigIdsP->find(eventNameNormalized) == m_commonContextEventToConfigIdsP->end())
            {
                m_commonContextEventToConfigIdsP->insert(std::pair<std::string, std::string>(eventNameNormalized, experimentIds));
            }
            else
            {
                (*m_commonContextEventToConfigIdsP)[eventNameNormalized] = experimentIds;
            }
        }
        else
        {
            m_commonContextEventToConfigIdsP->erase(eventNameNormalized);
        }
    }
}

void ContextFieldsProvider::SetAppId(std::string const& appId)
{
    setCommonField(COMMONFIELDS_APP_ID, appId);
}

void ContextFieldsProvider::SetAppVersion(std::string const& appVersion)
{
    setCommonField(COMMONFIELDS_APP_VERSION, appVersion);
}

void ContextFieldsProvider::SetAppLanguage(std::string const& appLanguage)
{
    setCommonField(COMMONFIELDS_APP_LANGUAGE, appLanguage);
}

void ContextFieldsProvider::SetDeviceId(std::string const& deviceId)
{
    setCommonField(COMMONFIELDS_DEVICE_ID, deviceId);
}

void ContextFieldsProvider::SetDeviceMake(std::string const& deviceMake)
{
    setCommonField(COMMONFIELDS_DEVICE_MAKE, deviceMake);
}

void ContextFieldsProvider::SetDeviceModel(std::string const& deviceModel)
{
    setCommonField(COMMONFIELDS_DEVICE_MODEL, deviceModel);
}

void ContextFieldsProvider::SetNetworkCost(NetworkCost networkCost)
{
    char const* value;

    switch (networkCost) {
        case NetworkCost_Unknown:
            value = "Unknown";
            break;

        case NetworkCost_Unmetered:
            value = "Unmetered";
            break;

        case NetworkCost_Metered:
            value = "Metered";
            break;

        case NetworkCost_OverDataLimit:
            value = "OverDataLimit";
            break;

        default:
            assert(!"Unknown NetworkCost enum value");
            value = "";
            break;
    }

    setCommonField(COMMONFIELDS_NETWORK_COST, value);
}

void ContextFieldsProvider::SetNetworkProvider(std::string const& networkProvider)
{
    setCommonField(COMMONFIELDS_NETWORK_PROVIDER, networkProvider);
}

void ContextFieldsProvider::SetNetworkType(NetworkType networkType)
{
    char const* value;

    switch (networkType) {
        case NetworkType_Unknown:
            value = "Unknown";
            break;

        case NetworkType_Wired:
            value = "Wired";
            break;

        case NetworkType_Wifi:
            value = "Wifi";
            break;

        case NetworkType_WWAN:
            value = "WWAN";
            break;

        default:
            assert(!"Unknown NetworkType enum value");
            value = "";
            break;
    }

    setCommonField(COMMONFIELDS_NETWORK_TYPE, value);
}

void ContextFieldsProvider::SetOsName(std::string const& osName)
{
    setCommonField(COMMONFIELDS_OS_NAME, osName);
}

void ContextFieldsProvider::SetOsVersion(std::string const& osVersion)
{
    setCommonField(COMMONFIELDS_OS_VERSION, osVersion);
}

void ContextFieldsProvider::SetOsBuild(std::string const& osBuild)
{
    setCommonField(COMMONFIELDS_OS_BUILD, osBuild);
}

void ContextFieldsProvider::SetUserId(std::string const& userId, PiiKind piiKind)
{
    EventProperty prop(userId, piiKind);
    setCommonField(COMMONFIELDS_USER_ID, prop);
}

void ContextFieldsProvider::SetUserMsaId(std::string const& userMsaId)
{
    setCommonField(COMMONFIELDS_USER_MSAID, userMsaId);
}

void ContextFieldsProvider::SetUserANID(std::string const& userANID)
{
    setCommonField(COMMONFIELDS_USER_ANID, userANID);
}

void ContextFieldsProvider::SetUserAdvertisingId(std::string const& userAdvertisingId)
{
    setCommonField(COMMONFIELDS_USER_ADVERTISINGID, userAdvertisingId);
}

void ContextFieldsProvider::SetUserLanguage(std::string const& language)
{
    setCommonField(COMMONFIELDS_USER_LANGUAGE, language);
}

void ContextFieldsProvider::SetUserTimeZone(std::string const& timeZone)
{
    setCommonField(COMMONFIELDS_USER_TIMEZONE, timeZone);
}

void ContextFieldsProvider::SetTicket(TicketType type, std::string const& ticketValue)
{
    if (!ticketValue.empty())
    {
        (*m_ticketsMapP)[type] = ticketValue;
    }
}

void ContextFieldsProvider::SetDeviceClass(std::string const& deviceCLass)
{
    setCommonField(COMMONFIELDS_DEVICE_CLASS, deviceCLass);
}

} ARIASDK_NS_END

// Copyright (c) Microsoft. All rights reserved.

#include "ContextFieldsProvider.hpp"
#include "Config.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {


ContextFieldsProvider::ContextFieldsProvider()
	: m_lockP(new std::mutex())
	, m_commonContextFieldsP(new std::map<std::string, EventProperty>())
	, m_customContextFieldsP(new std::map<std::string, EventProperty>())
{

}
ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider* parent)
  : m_lockP(new std::mutex())
   ,m_parent(parent)
   ,m_commonContextFieldsP( new std::map<std::string, EventProperty>())
   ,m_customContextFieldsP( new std::map<std::string, EventProperty>())
{
    if (!m_parent) {
        PAL::registerSemanticContext(this);
        setCommonField("act_session_id", PAL::generateUuidString());
    }
}

ContextFieldsProvider::ContextFieldsProvider(ContextFieldsProvider const& copy)
{
	m_lockP = new std::mutex();
	m_commonContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_commonContextFieldsP);
	m_customContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_customContextFieldsP);
}

ContextFieldsProvider& ContextFieldsProvider::operator=(ContextFieldsProvider const& copy)
{
	m_lockP = new std::mutex();
	m_commonContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_commonContextFieldsP);
	m_customContextFieldsP = new std::map<std::string, EventProperty>(*copy.m_customContextFieldsP);
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

void ContextFieldsProvider::writeToRecord(::AriaProtocol::Record& record) const
{
    if (m_parent) {
        m_parent->writeToRecord(record);
    }

    {
        std::lock_guard<std::mutex> lock(*m_lockP);

		std::map<std::string, AriaProtocol::PII> PIIExtensions;

        for (auto const& field : (*m_commonContextFieldsP))
		{
			if (field.second.piiKind != PiiKind_None)
			{
				//ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
				AriaProtocol::PII pii;
				pii.Kind = static_cast<AriaProtocol::PIIKind>(field.second.piiKind);
				pii.RawContent = field.second.to_string();
				pii.ScrubType = AriaProtocol::O365;
				PIIExtensions[field.first] = pii;
			}
			else 
			{
				std::vector<uint8_t> guid;
				uint8_t guid_bytes[16] = { 0 };

				switch (field.second.type) 
				{
				case EventProperty::TYPE_STRING:
					record.Extension[field.first] = field.second.to_string();
					break;
				case EventProperty::TYPE_INT64:
					record.TypedExtensionInt64[field.first] = field.second.as_int64;
					break;
				case EventProperty::TYPE_DOUBLE:
					record.TypedExtensionDouble[field.first] = field.second.as_double;
					break;
				case EventProperty::TYPE_TIME:
					record.TypedExtensionDateTime[field.first] = field.second.as_time_ticks.ticks;
					break;
				case EventProperty::TYPE_BOOLEAN:
					record.TypedExtensionBoolean[field.first] = field.second.as_bool;
					break;
				case EventProperty::TYPE_GUID:
				{
					GUID_t temp = field.second.as_guid;
					temp.to_bytes(guid_bytes);
					guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));
					record.TypedExtensionGuid[field.first] = guid;
					break;
				}
				default:
					// Convert all unknown types to string
					record.Extension[field.first] = field.second.to_string();
				}
			}
        }

        for (auto const& field : (*m_customContextFieldsP)) 
		{
			if (field.second.piiKind != PiiKind_None)
			{
				//ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
				AriaProtocol::PII pii;
				pii.Kind = static_cast<AriaProtocol::PIIKind>(field.second.piiKind);
				pii.RawContent = field.second.to_string();
				pii.ScrubType = AriaProtocol::O365;
				PIIExtensions[field.first] = pii;
			}
			else
			{
				std::vector<uint8_t> guid;
				uint8_t guid_bytes[16] = { 0 };

				switch (field.second.type)
				{
				case EventProperty::TYPE_STRING:
					record.Extension[field.first] = field.second.to_string();
					break;
				case EventProperty::TYPE_INT64:
					record.TypedExtensionInt64[field.first] = field.second.as_int64;
					break;
				case EventProperty::TYPE_DOUBLE:
					record.TypedExtensionDouble[field.first] = field.second.as_double;
					break;
				case EventProperty::TYPE_TIME:
					record.TypedExtensionDateTime[field.first] = field.second.as_time_ticks.ticks;
					break;
				case EventProperty::TYPE_BOOLEAN:
					record.TypedExtensionBoolean[field.first] = field.second.as_bool;
					break;
				case EventProperty::TYPE_GUID:
				{
					GUID_t temp = field.second.as_guid;
					temp.to_bytes(guid_bytes);
					guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));
					record.TypedExtensionGuid[field.first] = guid;
					break;
				}
				default:
					// Convert all unknown types to string
					record.Extension[field.first] = field.second.to_string();
				}
			}
        }

		if (PIIExtensions.size() > 0)
		{
			for (auto piiItem : PIIExtensions)
			{
				record.PIIExtensions[piiItem.first] = piiItem.second;
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
	setCommonField(COMMONFIELDS_APP_EXPERIMENTIDS,"");
}

void ContextFieldsProvider::SetAppExperimentIds(std::string const& appExperimentIds)
{
	setCommonField(COMMONFIELDS_APP_EXPERIMENTIDS, appExperimentIds);
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
			setCommonField(eventNameNormalized, experimentIds);
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


} ARIASDK_NS_END

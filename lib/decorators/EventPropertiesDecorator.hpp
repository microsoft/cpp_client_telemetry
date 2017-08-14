// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include <EventProperties.hpp>
#include "utils/Utils.hpp"
#include <algorithm>

namespace ARIASDK_NS_BEGIN {


class EventPropertiesDecorator : public DecoratorBase {
  public:
    EventPropertiesDecorator()
    {
    }

    bool decorate(::AriaProtocol::Record& record, EventPriority& priority, EventProperties const& eventProperties)
    {
        if (eventProperties.GetName().empty() && !record.EventType.empty()) {
            // OK, using some default set by earlier decorator.
        } 
        else 
        {
            if (!validateEventName(eventProperties.GetName())) {
                return false;
            }
            if (record.EventType.empty())
            {
                record.EventType = toLower(eventProperties.GetName());
            }
        }


		// get datetime in ISO 8601 format from record's timestamp
		record.Extension[COMMONFIELDS_EVENT_TIME] =  Microsoft::Applications::Telemetry::PAL::formatUtcTimestampMsAsISO8601(record.Timestamp);

		std::map<std::string, EventProperty> props;
		//messageToConvert.PopulateEventProperties(props);
		//messageToConvert.PopulateEventContextFields(props);
		// 1. Put event's properties into props
		// 2. Merge event context on top of it into props
		// 3. Populate Pii properties as record.PIIExtensions
		std::map<std::string, AriaProtocol::PII> PIIExtensions;
        std::map<std::string, AriaProtocol::CustomerContent> ccExtensions;

		for (auto &kv : eventProperties.GetProperties()) {

			if (!validatePropertyName(kv.first)) {
				return false;
			}
			auto k = kv.first;
			auto v = kv.second;
			if (v.piiKind != PiiKind_None ||
                v.ccKind != CustomerContentKind_None)
            {
                if (v.piiKind != PiiKind_None)
                {
                    //ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                    AriaProtocol::PII pii;
                    pii.Kind = static_cast<AriaProtocol::PIIKind>(v.piiKind);
                    pii.RawContent = v.to_string();
                    // ScrubType = 1 is the O365 scrubber which is the default behavior.
                    // pii.ScrubType = static_cast<PIIScrubber>(O365);
                    pii.ScrubType = AriaProtocol::O365;
                    PIIExtensions[k] = pii;
                    // 4. Send event's Pii context fields as record.PIIExtensions
                }
                else
                {
                    //ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                    AriaProtocol::CustomerContent cc;
                    cc.Kind = static_cast<AriaProtocol::CustomerContentKind>(v.ccKind);
                    cc.RawContent = v.to_string();
                    ccExtensions[k] = cc;
                    // 4. Send event's Pii context fields as record.PIIExtensions
                }
			}
			else {
				std::vector<uint8_t> guid;
				uint8_t guid_bytes[16] = { 0 };

				switch (v.type) {
				case EventProperty::TYPE_STRING:
					record.Extension[k] = v.to_string();
					break;
				case EventProperty::TYPE_INT64:
					record.TypedExtensionInt64[k] = v.as_int64;
					break;
				case EventProperty::TYPE_DOUBLE:
					record.TypedExtensionDouble[k] = v.as_double;
					break;
				case EventProperty::TYPE_TIME:
					record.TypedExtensionDateTime[k] = v.as_time_ticks.ticks;
					break;
				case EventProperty::TYPE_BOOLEAN:
					record.TypedExtensionBoolean[k] = v.as_bool;
					break;
				case EventProperty::TYPE_GUID:
					v.as_guid.to_bytes(guid_bytes);
					guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));
					record.TypedExtensionGuid[k] = guid;
					break;
				default:
					// Convert all unknown types to string
					record.Extension[k] = v.to_string();
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
        
        if (ccExtensions.size() > 0)
        {
            for (auto ccItem : ccExtensions)
            {
                record.CustomerContentExtensions[ccItem.first] = ccItem.second;
            }
        }        

		std::string sdkVersion = PAL::getSdkVersion();
		record.Extension[COMMONFIELDS_EVENT_SDKVERSION] = sdkVersion;
       
        if (eventProperties.GetPriority() != EventPriority_Unspecified) {
            priority = eventProperties.GetPriority();
        }

        return true;
    }

  protected:
  
  
};


} ARIASDK_NS_END

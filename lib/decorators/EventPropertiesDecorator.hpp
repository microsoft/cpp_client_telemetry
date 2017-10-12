// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include <EventProperties.hpp>
#include "CorrelationVector.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

namespace ARIASDK_NS_BEGIN {


class EventPropertiesDecorator : public DecoratorBase {
  public:
    EventPropertiesDecorator()
    {
    }

    bool decorate(::AriaProtocol::CsEvent& record, EventPriority& priority, EventProperties const& eventProperties)
    {
        if (eventProperties.GetName().empty()) {
            // OK, using some default set by earlier decorator.
        } 
        else 
        {
            if (!validateEventName(eventProperties.GetName())) {
                return false;
            }
        }

        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
                
        std::map<std::string, ::AriaProtocol::Value>& ext = record.data[0].properties;
        std::map<std::string, ::AriaProtocol::Value> extPartB;

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
                {   //ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                    AriaProtocol::PII pii;
                    pii.Kind = static_cast<AriaProtocol::PIIKind>(v.piiKind);
                    AriaProtocol::Value temp;

                    AriaProtocol::Attributes attrib;
                    attrib.pii.push_back(pii);


                    temp.attributes.push_back(attrib);
                    temp.stringValue = v.to_string();
                    if (v.dataCategory == DataCategory_PartB)
                    {
                        extPartB[k] = temp;
                    }
                    else
                    {
                        ext[k] = temp;
                    }
                }
                else
                {  //ARIASDK_LOG_DETAIL("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                    AriaProtocol::CustomerContent cc;
                    cc.Kind = static_cast<AriaProtocol::CustomerContentKind>(v.ccKind);
                    AriaProtocol::Value temp;
                   
                    AriaProtocol::Attributes attrib;
                    attrib.customerContent.push_back(cc);

                    temp.attributes.push_back(attrib);
                    temp.stringValue = v.to_string();
                    if (v.dataCategory == DataCategory_PartB)
                    {
                        extPartB[k] = temp;
                    }
                    else
                    {
                        ext[k] = temp;
                    }
                }
			}
			else {
				std::vector<uint8_t> guid;
				uint8_t guid_bytes[16] = { 0 };

				switch (v.type)
                {
				    case EventProperty::TYPE_STRING:
                    {
                        AriaProtocol::Value temp;
                        temp.stringValue = v.to_string();
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                        break;
                    }
				    case EventProperty::TYPE_INT64:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueInt64;
                        temp.longValue = v.as_int64;
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                        break;
                    }
				    case EventProperty::TYPE_DOUBLE:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueDouble;
                        temp.doubleValue = v.as_double;
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                        break;
                    }
				    case EventProperty::TYPE_TIME:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueDateTime;
                        temp.longValue = v.as_time_ticks.ticks;
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                        break;
                    }
				    case EventProperty::TYPE_BOOLEAN:
                    {
                        AriaProtocol::Value temp;
                        temp.type = ::AriaProtocol::ValueKind::ValueBool;
                        temp.longValue = v.as_bool;
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                        break;
                    }
				    case EventProperty::TYPE_GUID:
                    {
                        GUID_t temp = v.as_guid;
                        temp.to_bytes(guid_bytes);
                        guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));

                        AriaProtocol::Value tempValue;
                        tempValue.type = ::AriaProtocol::ValueKind::ValueGuid;
                        tempValue.guidValue.push_back( guid);
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = tempValue;
                        }
                        else
                        {
                            ext[k] = tempValue;
                        }
                        break;
                    }
				    default:
                    {
                        // Convert all unknown types to string
                        AriaProtocol::Value temp;
                        temp.stringValue = v.to_string();
                        if (v.dataCategory == DataCategory_PartB)
                        {
                            extPartB[k] = temp;
                        }
                        else
                        {
                            ext[k] = temp;
                        }
                    }
				}
			}
		}

        if (eventProperties.GetPriority() != EventPriority_Unspecified) {
            priority = eventProperties.GetPriority();
        }

        if (extPartB.size() > 0)
        {
            ::AriaProtocol::Data partBdata;
            partBdata.properties = extPartB;
            record.baseData.push_back(partBdata);
        // special case of CorrelationVector value
        if (ext.count(CorrelationVector::PropertyName) > 0)
        {
            AriaProtocol::Value cvValue = ext[CorrelationVector::PropertyName];

            if (cvValue.type == ::AriaProtocol::ValueKind::ValueString)
            {
                record.cV = cvValue.stringValue;
            }
            else
            {
                ARIASDK_LOG_DETAIL("CorrelationVector value type is invalid %u", cvValue.type);
            }

            ext.erase(CorrelationVector::PropertyName);
        }

        return true;
    }

  protected:
  
  
};


} ARIASDK_NS_END

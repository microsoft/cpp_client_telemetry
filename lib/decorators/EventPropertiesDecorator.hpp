// Copyright (c) Microsoft. All rights reserved.
#ifndef EVENTPROPERTIESDECORATOR_HPP
#define EVENTPROPERTIESDECORATOR_HPP

#include "IDecorator.hpp"
#include "EventProperties.hpp"
#include "CorrelationVector.hpp"
#include "utils/Utils.hpp"

#include <algorithm>
#include <map>
#include <string>

namespace ARIASDK_NS_BEGIN {


    class EventPropertiesDecorator : public DecoratorBase {

    public:
        EventPropertiesDecorator(ILogManager& owner) :
            DecoratorBase(owner) {};

        bool decorate(::CsProtocol::Record& record, EventLatency& latency, EventProperties const& eventProperties)
        {
            if (latency == EventLatency_Unspecified)
                latency = EventLatency_Normal;

            if (eventProperties.GetName().empty()) {
                // OK, using some default set by earlier decorator.
            }
            else
            {
                EventRejectedReason isValidEventName = validateEventName(eventProperties.GetName());
                if (isValidEventName != REJECTED_REASON_OK) {
                    LOG_ERROR("Invalid event properties!");
                    DebugEvent evt;
                    evt.type = DebugEventType::EVT_REJECTED;
                    evt.param1 = isValidEventName;
                    m_owner.DispatchEvent(evt);
                    return false;
                }
            }

            if (record.data.size() == 0)
            {
                ::CsProtocol::Data data;
                record.data.push_back(data);
            }

            record.popSample = eventProperties.GetPopSample();

            int64_t flags = eventProperties.GetPolicyBitFlags();
            // Pack flags the same way as Win 10 UTC is doing this
            flags = (flags & 0xffff) | ((flags & 0xffffffffffff0000) >> 8);

            if (EventPersistence_Critical == eventProperties.GetPersistence())
            {
                flags = flags | 0x02;
            }
            else
            {
                flags = flags | 0x01;
            }

            if (latency >= EventLatency_RealTime)
            {
                flags = flags | 0x0200;
            }
            else if (latency == EventLatency_CostDeferred)
            {
                flags = flags | 0x0300;
            }
            else
            {
                flags = flags | 0x0100;
            }
            record.flags = flags;

            std::map<std::string, ::CsProtocol::Value>& ext = record.data[0].properties;
            std::map<std::string, ::CsProtocol::Value> extPartB;

            for (auto &kv : eventProperties.GetProperties()) {

                EventRejectedReason isValidPropertyName = validatePropertyName(kv.first);
                if (isValidPropertyName != REJECTED_REASON_OK)
                {
                    DebugEvent evt;
                    evt.type = DebugEventType::EVT_REJECTED;
                    evt.param1 = isValidPropertyName;
                    m_owner.DispatchEvent(evt);
                    return false;
                }
                const auto &k = kv.first;
                const auto &v = kv.second;
                if (v.piiKind != PiiKind_None)
                {
                    if (v.piiKind == PiiKind::CustomerContentKind_GenericData)
                    {  //LOG_TRACE("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                        CsProtocol::CustomerContent cc;
                        cc.Kind = CsProtocol::CustomerContentKind::GenericContent;
                        CsProtocol::Value temp;

                        CsProtocol::Attributes attrib;
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
                    else
                    { //LOG_TRACE("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                        CsProtocol::PII pii;
                        pii.Kind = static_cast<CsProtocol::PIIKind>(v.piiKind);
                        CsProtocol::Value temp;

                        CsProtocol::Attributes attrib;
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
#if 0 /* v2 code */
                        if (v.piiKind != PiiKind_None)
                        {
                            //LOG_TRACE("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                            CsProtocol::PII pii;
                            pii.Kind = static_cast<CsProtocol::PIIKind>(v.piiKind);
                            pii.RawContent = v.to_string();
                            // ScrubType = 1 is the O365 scrubber which is the default behavior.
                            // pii.ScrubType = static_cast<PIIScrubber>(O365);
                            pii.ScrubType = CsProtocol::O365;
                            PIIExtensions[k] = pii;
                            // 4. Send event's Pii context fields as record.PIIExtensions
                        }
                        else
                        {
                            //LOG_TRACE("PIIExtensions: %s=%s (PiiKind=%u)", k.c_str(), v.to_string().c_str(), v.piiKind);
                            CsProtocol::CustomerContent cc;
                            cc.Kind = static_cast<CsProtocol::CustomerContentKind>(v.ccKind);
                            cc.RawContent = v.to_string();
                            ccExtensions[k] = cc;
                            // 4. Send event's Pii context fields as record.PIIExtensions
#endif
                    }
                }
                else {
                    std::vector<uint8_t> guid;
                    uint8_t guid_bytes[16] = { 0 };

                    switch (v.type)
                    {
                    case EventProperty::TYPE_STRING:
                    {
                        CsProtocol::Value temp;
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
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueInt64;
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
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueDouble;
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
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueDateTime;
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
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueBool;
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

                        CsProtocol::Value tempValue;
                        tempValue.type = ::CsProtocol::ValueKind::ValueGuid;
                        tempValue.guidValue.push_back(guid);
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
                    case EventProperty::TYPE_INT64_ARRAY:
                    {
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueArrayInt64;
                        temp.longArray.push_back(*v.as_longArray);
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
                    case EventProperty::TYPE_DOUBLE_ARRAY:
                    {
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueArrayDouble;
                        temp.doubleArray.push_back(*v.as_doubleArray);
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
                    case EventProperty::TYPE_STRING_ARRAY:
                    {
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueArrayString;
                        temp.stringArray.push_back(*v.as_stringArray);
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
                    case EventProperty::TYPE_GUID_ARRAY:
                    {
                        CsProtocol::Value temp;
                        temp.type = ::CsProtocol::ValueKind::ValueArrayGuid;

                        std::vector<std::vector<uint8_t>> values;
                        for (const auto& tempValue : *v.as_guidArray)
                        {
                            tempValue.to_bytes(guid_bytes);
                            guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));
                            values.push_back(guid);
                        }
                        temp.guidArray.push_back(values);
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
                    default:
                    {
                        // Convert all unknown types to string
                        CsProtocol::Value temp;
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

                if (extPartB.size() > 0)
                {
                    ::CsProtocol::Data partBdata;
                    partBdata.properties = extPartB;
                    record.baseData.push_back(partBdata);
                }
                // special case of CorrelationVector value
                if (ext.count(CorrelationVector::PropertyName) > 0)
                {
                    CsProtocol::Value cvValue = ext[CorrelationVector::PropertyName];

                    if (cvValue.type == ::CsProtocol::ValueKind::ValueString)
                    {
                        record.cV = cvValue.stringValue;
                    }
                    else
                    {
                        LOG_TRACE("CorrelationVector value type is invalid %u", cvValue.type);
                    }

                    ext.erase(CorrelationVector::PropertyName);
                }

                return true;
        }

    };

} ARIASDK_NS_END
#endif

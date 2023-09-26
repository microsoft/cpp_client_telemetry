//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef EVENTPROPERTIESDECORATOR_HPP
#define EVENTPROPERTIESDECORATOR_HPP

#include "IDecorator.hpp"
#include "EventProperties.hpp"
#include "CorrelationVector.hpp"
#include "utils/Utils.hpp"

#include <algorithm>
#include <map>
#include <string>

namespace MAT_NS_BEGIN {

// Bit remapping has to happen on bits passed via API surface.
// Ref CS2.1+ : https://osgwiki.com/wiki/CommonSchema/flags
// #define MICROSOFT_EVENTTAG_MARK_PII 0x08000000
#define RECORD_FLAGS_EVENTTAG_MARK_PII 0x00080000
// #define MICROSOFT_EVENTTAG_HASH_PII 0x04000000
#define RECORD_FLAGS_EVENTTAG_HASH_PII 0x00100000
// #define MICROSOFT_EVENTTAG_DROP_PII 0x02000000
#define RECORD_FLAGS_EVENTTAG_DROP_PII 0x00200000

    class EventPropertiesDecorator : public IDecorator
    {
    protected:
        std::string randomLocalId;

        ILogManager& m_owner;
        bool decorate(::CsProtocol::Record&) override
        {
            return false;
        }

    public:
        EventPropertiesDecorator(ILogManager& owner) :
            m_owner(owner)
        {
            //
            // Random local deviceId must be used for events tagged with Pii DROP EventTag.
            // Since we generate a new random id for every SDK session - products tagging
            // their telemetry events with Pii DROP tag cannot use ext.device.localId field
            // in Kusto for DAU/MAU user engagement metrics.
            //
            // It would have been more logical to adjust the Pii flags spec to require all
            // devices running with Pii DROP flag to the same identical bogus {deadbeef-...}
            // id. That would have allowed the apps to continue using ext.device.localId for
            // engagement metrics, estimating their population size using that field.
            //
            randomLocalId = "r:";
            randomLocalId+= PAL::generateUuidString();
        };

        void dropPiiPartA(::CsProtocol::Record& record)
        {
            // MICROSOFT_EVENTTAG_DROP_PII tag functionality reference:
            // https://osgwiki.com/wiki/Telemetry#De-Identification_of_Telemetry_Events
            //
            // Drop Pii EventTag scrubs Part A Pii data client-side.
            // The flag has no effect on Part C Pii data.
            //

            // clear tickets because these provide a way to identify the end-user
            record.extProtocol[0].ticketKeys.clear();

            // clean Pii from Device extension
            record.extDevice[0].localId = randomLocalId;
            record.extDevice[0].authId.clear();
            record.extDevice[0].authSecId.clear();
            record.extDevice[0].id.clear();

            // clean Pii from User extension
            record.extUser[0].localId.clear();
            record.extUser[0].authId.clear();
            record.extUser[0].id.clear();

            // clean epoch and seq + installId
            record.extSdk[0].seq = 0;
            record.extSdk[0].epoch.clear();
            record.extSdk[0].installId.clear();

            // clear correlation vector
            record.cV = "";
        }

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

            auto timestamp = eventProperties.GetTimestamp();
            if (timestamp != 0)
                // convert timestamp in millis to ticks and add ticks for UTC time 0.
                record.time = timestamp * 10000 + 0x89F7FF5F7B58000ULL;

            record.popSample = eventProperties.GetPopSample();

            // API surface tags('flags') are different from on-wire record.flags
            int64_t tags = eventProperties.GetPolicyBitFlags();
            int64_t flags = 0;

            // We must remap from one bitfield set to another, no way to bit-shift :(
            // At the moment 1DS SDK in direct upload mode supports DROP and MARK tags only:
            flags |= (tags & MICROSOFT_EVENTTAG_MARK_PII) ? RECORD_FLAGS_EVENTTAG_MARK_PII : 0;
            flags |= (tags & MICROSOFT_EVENTTAG_DROP_PII) ? RECORD_FLAGS_EVENTTAG_DROP_PII : 0;

            // Caller asked to drop Pii from Part A of that event
            bool tagDropPii = bool(tags & MICROSOFT_EVENTTAG_DROP_PII);

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

            // scrub if MICROSOFT_EVENTTAG_DROP_PII is set
            if (tagDropPii)
            {
                dropPiiPartA(record);
            }

            return true;
        }

    };

} MAT_NS_END
#endif


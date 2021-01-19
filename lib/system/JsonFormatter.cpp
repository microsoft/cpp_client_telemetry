//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "JsonFormatter.hpp"
#include "CorrelationVector.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace MAT_NS_BEGIN
{

    JsonFormatter::JsonFormatter()
    {

    }

    void addExtApp(json& object, std::vector<::CsProtocol::App>& extApp)
    {
        if (!extApp[0].id.empty())
        {
            object["ext"]["extApp"]["name"] = extApp[0].id;
        }
        if (!extApp[0].expId.empty())
        {
            object["ext"]["extApp"]["expId"] = extApp[0].id;
        }
    }

    void addExtNet(json& object, std::vector<::CsProtocol::Net>& extNet)
    {
        if (!extNet[0].cost.empty())
        {
            object["ext"]["extNet"]["cost"] = extNet[0].cost;
        }
        if (!extNet[0].type.empty())
        {
            object["ext"]["extNet"]["type"] = extNet[0].type;
        }
    }

    void addData(json& object, std::vector<::CsProtocol::Data>& data)
    {
        std::vector<::CsProtocol::Data>::const_iterator it;
        for (it = data.begin(); it != data.end(); ++it)
        {
            std::map<std::string, CsProtocol::Value>::const_iterator mapIt;
            for (mapIt = it->properties.begin(); mapIt != it->properties.end(); ++mapIt)
            {
                switch (mapIt->second.type)
                {
                case CsProtocol::ValueKind::ValueInt64:
                    object["data"][mapIt->first] = mapIt->second.longValue;
                    break;
                case CsProtocol::ValueKind::ValueUInt64:
                    object["data"][mapIt->first] = mapIt->second.longValue;
                    break;
                case CsProtocol::ValueKind::ValueInt32:
                    object["data"][mapIt->first] = static_cast<int32_t>(mapIt->second.longValue);
                    break;
                case CsProtocol::ValueKind::ValueUInt32:
                    object["data"][mapIt->first] = static_cast<uint32_t>(mapIt->second.longValue);
                    break;
                case CsProtocol::ValueKind::ValueBool:
                {
                    uint8_t temp = static_cast<uint8_t>(mapIt->second.longValue);
                    object["data"][mapIt->first] = temp;
                    break;
                }
                case CsProtocol::ValueKind::ValueDateTime:
                {
                    object["data"][mapIt->first] = mapIt->second.longValue;
                    break;
                }
                case CsProtocol::ValueKind::ValueArrayInt64:
                    object["data"][mapIt->first] = mapIt->second.longArray;
                    break;
                case CsProtocol::ValueKind::ValueArrayUInt64:
                    object["data"][mapIt->first] = mapIt->second.longArray;
                    break;
                case CsProtocol::ValueKind::ValueArrayInt32:
                    object["data"][mapIt->first] = mapIt->second.longArray;
                    break;
                case CsProtocol::ValueKind::ValueArrayUInt32:
                    object["data"][mapIt->first] = mapIt->second.longArray;
                    break;
                case CsProtocol::ValueKind::ValueArrayBool:
                case CsProtocol::ValueKind::ValueArrayDateTime:
                    break;
                case CsProtocol::ValueKind::ValueDouble:
                {
                    object["data"][mapIt->first] = mapIt->second.doubleValue;
                    break;
                }
                case CsProtocol::ValueKind::ValueArrayDouble:
                {
                    object["data"][mapIt->first] = mapIt->second.doubleArray;
                    break;
                }
                case CsProtocol::ValueKind::ValueString:
                {
                    object["data"][mapIt->first] = mapIt->second.stringValue;
                    break;
                }
                case CsProtocol::ValueKind::ValueArrayString:
                {
                    object["data"][mapIt->first] = mapIt->second.stringArray;
                    break;
                }
                case CsProtocol::ValueKind::ValueGuid:
                {
                    if (mapIt->second.guidValue.size() > 0)
                    {
                        /*GUID temp = GUID_t::convertUintVectorToGUID(mapIt->second.guidValue[0]);
                        myJson["data"][mapIt->first] = temp;*/
                    }
                    break;
                }
                default:
                {
                   LOG_WARN("Unsupported type %d", static_cast<int32_t>(mapIt->second.type));
                   break;
                }
                }
            }
        }
    }

    std::string JsonFormatter::getJsonFormattedEvent(IncomingEventContextPtr const& event)
    {
        json ans = json::object();
        ::CsProtocol::Record* source = event->source;
        ans["ver"] = source->ver;
        ans["name"] = source->name;
        if (source->time) ans["time"] = source->time;
        std::string iKey("P-ARIA-");
        iKey.append(event->record.tenantToken);
        ans["iKey"] = iKey;
        if (!source->cV.empty())
            ans[CorrelationVector::PropertyName] = source->cV;
        if (source->data[0].properties.find(COMMONFIELDS_EVENT_PRIVTAGS) != source->data[0].properties.end()) {
            ans["ext"]["metadata"]["privTags"] = source->data[0].properties[COMMONFIELDS_EVENT_PRIVTAGS].longValue;
            source->data[0].properties.erase(COMMONFIELDS_EVENT_PRIVTAGS);
        }
        addExtApp(ans, source->extApp);
        addExtNet(ans, source->extNet);

        std::string userLocalId = source->extUser[0].localId;
        if (!userLocalId.empty())
        {
            std::string userId("e:");
            userId.append(userLocalId);
            ans["ext"]["user"]["localId"] = userId;
        }
        std::string userLanguage = source->extUser[0].locale;
        if (!userLanguage.empty())
        {
            ans["ext"]["os"]["locale"] = userLanguage;
        }

        source->data[0].properties.erase(COMMONFIELDS_USER_MSAID);
        source->data[0].properties.erase(COMMONFIELDS_DEVICE_ID);
        source->data[0].properties.erase(COMMONFIELDS_OS_NAME);
        source->data[0].properties.erase(COMMONFIELDS_OS_VERSION);
        source->data[0].properties.erase(COMMONFIELDS_OS_BUILD);
        source->data[0].properties.erase(COMMONFIELDS_EVENT_TIME);
        source->data[0].properties.erase(COMMONFIELDS_USER_ANID);
        source->data[0].properties.erase(COMMONFIELDS_APP_VERSION);
        source->data[0].properties.erase(COMMONFIELDS_EVENT_NAME);
        source->data[0].properties.erase(COMMONFIELDS_EVENT_INITID);
        source->data[0].properties.erase(COMMONFIELDS_EVENT_PRIVTAGS);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPRODUCERID);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGCATEGORY);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA1);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA2);
        source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA3);

        addData(ans, source->ext);
        addData(ans, source->data);
        addData(ans, source->baseData);
        return ans.dump(4);
    }

} MAT_NS_END

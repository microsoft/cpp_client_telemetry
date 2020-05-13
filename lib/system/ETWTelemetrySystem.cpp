// Copyright (c) Microsoft. All rights reserved.

#include "mat/config.h"
#ifdef HAVE_MAT_ETW

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
// needed for Unit Testing with krabs.hpp
#pragma warning( disable : 4018 )
#endif

#include "ETWTelemetrySystem.hpp"

#include "pal/PAL.hpp"
#include "utils/Utils.hpp"
#include "CommonFields.h"

#include "CorrelationVector.hpp"

// krabs.hpp requires this definition of min macro from Windows.h
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

using namespace tld;

namespace ARIASDK_NS_BEGIN
{
    const char* const ARIA_METADATA_COMMON_TEXT = "n:";
    const char* const ARIA_METADATA_TYPE_TEXT = ";t:";
    const char* const IKEY_PRE_TEXT = "P-ARIA-";

    const char* const UTC_TYPE = "recType";
    const char* const UTC_PART_A_EXTS_BC = "PartAExt_bc";
    const char* const UTC_MAKE = "make";
    const char* const UTC_MODEL = "model";
    const char* const UTC_TIMEZONE = "tz";
    const char* const UTC_APP_LANG = "appLang";
    const char* const UTC_BC_SOURCE = "source";
    const char* const UTC_BC_ADVERTISEMENT_ID = "advertisingId";
    const char* const UTC_APP_EXPERIMENT_IDS = "PartA_Ext_App_ExpId";
    const char* const UTC_APP_ETAG = "expEtag";
    const char* const UTC_PART_A_EXTS_ARIA = "PartAExt_aria";
    const char* const UTC_LIB_VERSION = "libVer";

    const char* const CS_EXT_IKEY = "PartA_iKey";
    const char* const CS_EXT_APP_NAME = "PartA_Ext_App_Name";
    const char* const CS_EXT_NET_PROVIDER = "PartA_Ext_Net_Provider";
    const char* const CS_EXT_NET_COST = "PartA_Ext_Net_Cost";
    const char* const CS_EXT_NET_TYPE = "PartA_Ext_Net_Type";
    const char* const CS_EXT_APP_SEQ_NUM = "PartA_Ext_App_SeqNum";
    const char* const CS_EXT_APP_SESSION_ID = "sesId";
    const char* const CS_EXT_RECORD_TIMESTAMP = "evtTime";
    const char* const CS_EXT_APP_ASID = "PartA_Ext_App_AsId";
    const char* const CS_EXT_APP_USERID = "PartA_Ext_App_UserId";
    const char* const CS_EXT_OS_LOCALE = "PartA_Ext_Os_Locale";
    const char* const CS_EXT_USER_AUTH_ID = "PartA_Ext_User_AuthId";
    const char* const CS_EXT_ARIA_METADATA = "PartA_Ext_AriaMD";
    const char* const CS_EXT_ARIA_METADATA_FIELDS = "fields";
    const char* const CS_EXT_METADATA_PRIVTAGS = "PartA_PrivTags";

    enum EventPropertyType : UINT16
    {
        EventPropertyTypeString = 0,    // (Default, not emitted in "t" field)
        EventPropertyTypeBool = 1,      // 32 bit
        EventPropertyTypeInt64 = 2,     // 64 bit
        EventPropertyTypeDouble = 3,    // double
        EventPropertyTypeDateTime = 4,  // 64 bit
        EventPropertyTypeGuid = 5       // 128 bit
    };

    static const unsigned int LargeEventSizeKB = 62;

    ETWTelemetrySystem::ETWTelemetrySystem(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher)
        :
        TelemetrySystemBase(logManager, runtimeConfig, taskDispatcher)
    {
        this->sending >> this->incomingEventPrepared;
    }

    ETWTelemetrySystem::~ETWTelemetrySystem()
    {
    }

    void ETWTelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
    {
        //send event to UTC here
        if (0 != sendEventToETW(event))
        {
            DispatchEvent(DebugEventType::EVT_SEND_FAILED);
        }
    }

    ETWProviderData& ETWTelemetrySystem::getProviderForToken(const std::string& token)
    {
        std::string tenant = tenantKeyToId(token);
        auto it = providers.find(tenant);
        if (it == providers.end())
        {
            return registerProviderForTenant(tenant);
        }
        return it->second;
    }

    ETWProviderData& ETWTelemetrySystem::registerProviderForTenant(const std::string& tenant)
    {
        GUID_t guid(tenant.c_str());
        // create data in the map
        ETWProviderData& data = providers[tenant];
        to_GUID(guid, data.providerGuid);

        // TODO: currently we do not allow to specify a custom group GUID
        GUID providerGroupGuid = GUID_NULL;

        tld::ProviderMetadataBuilder<std::vector<BYTE>> providerMetaBuilder(data.providerMetaVector);

        // Use Tenant ID as provider Name
        providerMetaBuilder.Begin(tenant.c_str());
        providerMetaBuilder.AddTrait(ProviderTraitType::ProviderTraitGroupGuid, (void*)&providerGroupGuid, sizeof(GUID));
        providerMetaBuilder.End();

        REGHANDLE hProvider = 0;
        if (0 != RegisterProvider(&hProvider, &data.providerGuid, data.providerMetaVector.data()))
        {
            LOG_WARN("Unable to RegisterProvider guid=%s", guid.to_string().c_str());
        }

        data.providerHandle = hProvider;

        // We always return an entry, but the client code should check whether the hProvider handle is valid or not
        return data;
    }

    status_t ETWTelemetrySystem::unregisterProvider(ETWProviderData& data)
    {
        // TODO: [MG] - not implemented yet
        UNREFERENCED_PARAMETER(data);
        return status_t::STATUS_ENOTSUP;
    }

    int ETWTelemetrySystem::sendEventToETW(IncomingEventContextPtr const& eventCtx)
    {
        ETWProviderData& providerData = getProviderForToken(eventCtx->record.tenantToken);

        UINT32 eventTags = MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE;
        if (eventCtx->record.persistence > EventPersistence::EventPersistence_Normal)
        {
            eventTags = MICROSOFT_EVENTTAG_CRITICAL_PERSISTENCE;
        }
        eventTags |= static_cast<UINT32>(eventCtx->policyBitFlags);

        std::vector<std::string> MD;
        std::vector<BYTE> byteVector;
        std::vector<BYTE> byteDataVector;
        tld::EventMetadataBuilder<std::vector<BYTE>> builder(byteVector);
        tld::EventDataBuilder<std::vector<BYTE>> dbuilder(byteDataVector);

        std::string eventName = eventCtx->source->name;
        if (eventName.empty())
        {
            eventName = "Unspecified";
        }

        builder.Begin(eventName.c_str(), eventTags);

        // TODO: [MG] - is ext.data mandatory - can event contain no data, just name?
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_MAKE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_MODEL);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_TIMEZONE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_LANGUAGE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SOURCE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_ADVERTISINGID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_EXPERIMENTETAG);

        // Correlation Vector is passed to UTC as a regular top-level field with a reserved name.
        if (!eventCtx->source->cV.empty())
        {
            builder.AddField(CorrelationVector::PropertyName, TypeUtf8String);
            dbuilder.AddString(eventCtx->source->cV.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_EVENT_PRIVTAGS) != eventCtx->source->data[0].properties.end())
        {
            // UTC mode sends privacy tag in ext.metadata.privTags
            builder.AddField(CS_EXT_METADATA_PRIVTAGS, TypeUInt64);
            int64_t value = eventCtx->source->data[0].properties[COMMONFIELDS_EVENT_PRIVTAGS].longValue;
            dbuilder.AddValue(value);
        }

        //PartA_Exts_CommonFields
        builder.AddField(CS_EXT_IKEY, TypeUtf8String);
        std::string iKey(IKEY_PRE_TEXT);
        iKey.append(eventCtx->record.tenantToken);
        dbuilder.AddString(iKey.c_str());

        // Optional ext.app extension
        if (eventCtx->source->extApp.size())
        {
            std::string appInfoAppName = eventCtx->source->extApp[0].id;
            if (!appInfoAppName.empty())
            {
                builder.AddField(CS_EXT_APP_NAME, TypeUtf8String);
                dbuilder.AddString(appInfoAppName.c_str());
            }

            std::string appInfoExpIds = eventCtx->source->extApp[0].expId;
            if (!appInfoExpIds.empty())
            {
                builder.AddField(UTC_APP_EXPERIMENT_IDS, TypeUtf8String);
                dbuilder.AddString(appInfoExpIds.c_str());
            }
        }

        if (eventCtx->source->extNet.size())
        {
            std::string deviceInfoNetworkProvider = eventCtx->source->extNet[0].provider;
            if (!deviceInfoNetworkProvider.empty())
            {
                builder.AddField(CS_EXT_NET_PROVIDER, TypeUtf8String);
                dbuilder.AddString(deviceInfoNetworkProvider.c_str());
            }

            std::string deviceInfoNetworkCost = eventCtx->source->extNet[0].cost;
            if (!deviceInfoNetworkCost.empty())
            {
                builder.AddField(CS_EXT_NET_COST, TypeUtf8String);
                dbuilder.AddString(deviceInfoNetworkCost.c_str());
            }

            std::string deviceInfoNetworkType = eventCtx->source->extNet[0].type;
            if (!deviceInfoNetworkType.empty())
            {
                builder.AddField(CS_EXT_NET_TYPE, TypeUtf8String);
                dbuilder.AddString(deviceInfoNetworkType.c_str());
            }
        }

        if (eventCtx->source->extUser.size())
        {
            std::string sdkUserId(eventCtx->source->extUser[0].localId);
            if (!sdkUserId.empty())
            {
                std::string userId("e:");
                userId.append(sdkUserId);
                builder.AddField(CS_EXT_APP_USERID, TypeUtf8String);
                dbuilder.AddString(userId.c_str());
            }

            std::string userInfoLanguage = eventCtx->source->extUser[0].locale;
            if (!userInfoLanguage.empty())
            {
                builder.AddField(CS_EXT_OS_LOCALE, TypeUtf8String);
                dbuilder.AddString(userInfoLanguage.c_str());
            }
        }

        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_MSAID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_ID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_NAME);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_VERSION);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_BUILD);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_TIME);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_ANID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_VERSION);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_NAME);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_INITID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_PRIVTAGS);

        //"Extension"
        PutData(eventCtx->source->ext, MD, builder, dbuilder);
        PutData(eventCtx->source->baseData, MD, builder, dbuilder);
        PutData(eventCtx->source->data, MD, builder, dbuilder);

        //PartA_Ext_ariaMD
        if (MD.size() > 0)
        {
            auto structPartAExtsMD = builder.AddStruct(CS_EXT_ARIA_METADATA);
            structPartAExtsMD.AddFieldFixedArray(CS_EXT_ARIA_METADATA_FIELDS, static_cast<UINT16>(MD.size()), TypeUtf8String);
            std::vector<std::string>::iterator iterMD;
            for (iterMD = MD.begin(); iterMD != MD.end(); ++iterMD)
            {
                dbuilder.AddString(iterMD->c_str());
            }
        }

        if (!builder.End())  // Returns false if the metadata is too large.
        {
            return -1;  // if event is too big for UTC to handle
        }

        tld::EventDescriptor eventDescriptor;

        if (eventCtx->policyBitFlags & MICROSOFT_KEYWORD_CRITICAL_DATA)
        {
            eventDescriptor.Keyword = MICROSOFT_KEYWORD_CRITICAL_DATA;
        }
        else if (eventCtx->policyBitFlags & MICROSOFT_KEYWORD_TELEMETRY)
        {
            eventDescriptor.Keyword = MICROSOFT_KEYWORD_TELEMETRY;
        }
        else
        {
            eventDescriptor.Keyword = MICROSOFT_KEYWORD_MEASURES;
        }

        EVENT_DATA_DESCRIPTOR pDataDescriptors[3];
        EventDataDescCreate(&pDataDescriptors[2], byteDataVector.data(), static_cast<ULONG>(byteDataVector.size()));

        // Event size detection is needed
        int64_t eventByteSize = byteDataVector.size() + byteVector.size();
        int64_t eventKBSize = (eventByteSize + 1024 - 1) / 1024;
        bool isLargeEvent = eventKBSize >= LargeEventSizeKB;

        MAT::GUID_t guid(providerData.providerGuid);
        if (!isLargeEvent)
        {
            LOG_INFO("Sending ETW event Id=%u, provider GUID=%s", (uint16_t)(eventDescriptor.Id), guid.to_string().c_str());
            HRESULT writeResponse = tld::WriteEvent(
                providerData.providerHandle,
                eventDescriptor,
                providerData.providerMetaVector.data(),
                byteVector.data(),
                3,
                pDataDescriptors);
            if (writeResponse == 0)
                return 0;
            else if (writeResponse == HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW))
                isLargeEvent = true;
            else
                return -1;
        }

        LOG_WARN("Failed to send ETW event Id=%u, provider GUID=%s: event is too big.",
            (uint16_t)(eventDescriptor.Id),
            guid.to_string().c_str());
        // Not supported
        return -1;
    }

    void ETWTelemetrySystem::PutData(std::vector<::CsProtocol::Data>& ext,
        std::vector<std::string>& MD,
        tld::EventMetadataBuilder<std::vector<BYTE>>& builder,
        tld::EventDataBuilder<std::vector<BYTE>>& dbuilder)
    {
        if (ext.size() > 0)
        {
            std::vector<::CsProtocol::Data>::const_iterator iterExtension;
            for (iterExtension = ext.begin(); iterExtension != ext.end(); ++iterExtension)
            {
                std::map<std::string, CsProtocol::Value>::const_iterator iterValue;
                for (iterValue = iterExtension->properties.begin(); iterValue != iterExtension->properties.end(); ++iterValue)
                {
                    std::string name = iterValue->first;
                    std::string metadata(ARIA_METADATA_COMMON_TEXT);
                    metadata.append(name.c_str());

                    // If pii is present, do not append ";t:" to ariaMD
                    if (!(iterValue->second.attributes.size() > 0 && iterValue->second.attributes[0].pii.size() > 0))
                    {
                        metadata.append(ARIA_METADATA_TYPE_TEXT);
                    }

                    bool addMetaData = true;
                    bool error = false;

                    switch (iterValue->second.type)
                    {
                    case CsProtocol::ValueKind::ValueInt64:
                    case CsProtocol::ValueKind::ValueUInt64:
                    case CsProtocol::ValueKind::ValueInt32:
                    case CsProtocol::ValueKind::ValueUInt32:
                    {
                        builder.AddField(name.c_str(), TypeInt64);
                        INT64 temp = iterValue->second.longValue;
                        dbuilder.AddBytes(&temp, sizeof(INT64));

                        metadata.append(std::to_string(EventPropertyTypeInt64));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueBool:
                    {
                        builder.AddField(name.c_str(), TypeBool8);
                        UINT8 temp = static_cast<UINT8>(iterValue->second.longValue);
                        dbuilder.AddByte(temp);

                        metadata.append(std::to_string(EventPropertyTypeBool));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueDateTime:
                    {
                        builder.AddField(name.c_str(), TypeUInt64);
                        UINT64 temp = iterValue->second.longValue;
                        dbuilder.AddBytes(&temp, sizeof(UINT64));

                        metadata.append(std::to_string(EventPropertyTypeDateTime));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueArrayInt64:
                    case CsProtocol::ValueKind::ValueArrayUInt64:
                    case CsProtocol::ValueKind::ValueArrayInt32:
                    case CsProtocol::ValueKind::ValueArrayUInt32:
                    case CsProtocol::ValueKind::ValueArrayBool:
                    case CsProtocol::ValueKind::ValueArrayDateTime:
                    {
                        break;
                    }
                    case CsProtocol::ValueKind::ValueDouble:
                    {
                        builder.AddField(name.c_str(), TypeDouble);
                        double temp = iterValue->second.doubleValue;
                        dbuilder.AddBytes(&temp, sizeof(double));

                        metadata.append(std::to_string(EventPropertyTypeDouble));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueArrayDouble:
                    {
                        break;
                    }
                    case CsProtocol::ValueKind::ValueString:
                    {
                        addMetaData = false;
                        builder.AddField(name.c_str(), TypeUtf8String);
                        dbuilder.AddString(iterValue->second.stringValue.c_str());
                        break;
                    }
                    case CsProtocol::ValueKind::ValueArrayString:
                    {
                        addMetaData = false;
                        builder.AddField(name.c_str(), TypeUtf8String);
                        dbuilder.AddString(iterValue->second.stringValue.c_str());
                        break;
                    }
                    case CsProtocol::ValueKind::ValueGuid:
                    {
                        if (iterValue->second.guidValue.size() > 0)
                        {
                            GUID temp = GUID_t::convertUintVectorToGUID(iterValue->second.guidValue[0]);
                            if (false == error)
                            {
                                builder.AddField(name.c_str(), TypeGuid);
                                dbuilder.AddBytes(&temp, sizeof(GUID));

                                metadata.append(std::to_string(EventPropertyTypeGuid));
                            }
                        }
                        break;
                    }
                    case CsProtocol::ValueKind::ValueArrayGuid:
                    {
                        break;
                    }
                    }
                    if (false == error)
                    {
                        if (iterValue->second.attributes.size() > 0)
                        {
                            if (iterValue->second.attributes[0].pii.size() > 0)
                            {
                                UINT8 ScrubType = 1;  //PIIScrubber{ NotSet = 0, O365 = 1, SkypeBI = 2, SkypeData = 3} always set to O365
                                metadata.append(";s:");
                                metadata.append(std::to_string(ScrubType));

                                UINT8 Kind = static_cast<UINT8>(iterValue->second.attributes[0].pii[0].Kind);
                                metadata.append(";k:");
                                metadata.append(std::to_string(Kind));
                            }
                            MD.push_back(metadata);
                        }
                        else
                        {
                            if (addMetaData)
                            {
                                MD.push_back(metadata);
                            }
                        }
                    }
                }
            }
        }
    }

} ARIASDK_NS_END
#endif

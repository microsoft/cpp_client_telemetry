#include "mat/config.h"
#ifdef HAVE_MAT_ETW
// Copyright (c) Microsoft. All rights reserved.
#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif

#pragma comment(lib, "windowsapp")
#pragma comment(lib, "runtimeobject" )

#include "ETWTelemetrySystem.hpp"

#include "CorrelationVector.hpp"
#include "pal/PAL.hpp"
#include "utils/Utils.hpp"
#include "MicrosoftTelemetry.h"

#include "CommonFields.h"

using namespace tld;

namespace ARIASDK_NS_BEGIN
{

    static const unsigned char InvalidHexDigit = 0xFF;

    static const unsigned int LargeEventSizeKB = 62;

    unsigned char getHexToInt(char ch)
    {
        unsigned char temp = 0;
        switch (ch)
        {
        case '0':
            temp = 0; break;
        case '1':
            temp = 1; break;
        case '2':
            temp = 2; break;
        case '3':
            temp = 3; break;
        case '4':
            temp = 4; break;
        case '5':
            temp = 5; break;
        case '6':
            temp = 6; break;
        case '7':
            temp = 7; break;
        case '8':
            temp = 8; break;
        case '9':
            temp = 9; break;
        case 'a':
            temp = 10; break;
        case 'A':
            temp = 10; break;
        case 'b':
            temp = 11; break;
        case 'B':
            temp = 11; break;
        case 'c':
            temp = 12; break;
        case 'C':
            temp = 12; break;
        case 'd':
            temp = 13; break;
        case 'D':
            temp = 13; break;
        case 'e':
            temp = 14; break;
        case 'E':
            temp = 14; break;
        case 'f':
            temp = 15; break;
        case 'F':
            temp = 15; break;
        default:
            temp = InvalidHexDigit; break;
        }
        return temp;
    }

    GUID ConvertGuidStringToGUID(std::string const& guidString, bool& error)
    {
        GUID temp;

        temp.Data1 = 0;
        temp.Data2 = 0;
        temp.Data3 = 0;

        int index = 0;
        for (; index < 8; index++)
        {
            temp.Data4[index] = 0;
        }

        if (guidString.empty() || guidString.size() != 32)
        {
            error = true;
            return temp;
        }

        unsigned long  Data1 = 0;
        unsigned short Data2 = 0;
        unsigned short Data3 = 0;
        unsigned char  Data4[8] = { 0 };
        index = 0;
        for (; index < 8; index++)
        {
            unsigned char tempInt = getHexToInt(guidString[index]);
            if (InvalidHexDigit == tempInt)
            {
                error = true;
                return temp;
            }

            Data1 = Data1 * 16 + tempInt;
        }

        for (; index < 12; index++)
        {
            unsigned char tempInt = getHexToInt(guidString[index]);
            if (InvalidHexDigit == tempInt)
            {
                error = true;
                return temp;
            }

            Data2 = Data2 * 16 + tempInt;
        }

        for (; index < 16; index++)
        {
            unsigned char tempInt = getHexToInt(guidString[index]);
            if (InvalidHexDigit == tempInt)
            {
                error = true;
                return temp;
            }

            Data3 = Data3 * 16 + tempInt;
        }

        int data4Index = 0;
        unsigned char data4value = 0;
        bool resetValue = true;
        unsigned char tempInt = 0;
        for (; index < 32; index++)
        {
            if (resetValue)
            {
                tempInt = 0;
                resetValue = false;
            }
            else
            {
                resetValue = true;
            }

            tempInt = getHexToInt(guidString[index]);
            if (-1 == tempInt)
            {
                error = true;
                return temp;
            }

            data4value = data4value * 16 + tempInt;

            if (resetValue)
            {
                Data4[data4Index] = data4value;
                data4value = 0;
                data4Index++;
            }
        }

        temp.Data1 = Data1;
        temp.Data2 = Data2;
        temp.Data3 = Data3;

        index = 0;
        for (; index < 8; index++)
        {
            temp.Data4[index] = Data4[index];
        }

        return temp;
    }

    HRESULT ETWTelemetrySystem::RPCWriteEvent(
        GUID providerGuid,
        USHORT providerMetaSize,
        const BYTE* pProviderMeta,
        USHORT traceLoggingMetaSize,
        const BYTE* pTraceLoggingMeta,
        ULONG traceLoggingPayloadSize,
        const BYTE* pTraceLoggingPayload,
        PCEVENT_DESCRIPTOR pEventDescriptor,
        LPCGUID pActivityId,
        LPCGUID pRelatedActivityId)
    {
        if (m_sendTraceLogging2Fn != nullptr)
        {
            return m_sendTraceLogging2Fn(providerGuid, providerMetaSize, pProviderMeta, traceLoggingMetaSize, pTraceLoggingMeta,
                traceLoggingPayloadSize, pTraceLoggingPayload, pEventDescriptor, pActivityId, pRelatedActivityId, 0);
        }
        else if (m_sendTraceLoggingFn != nullptr)
        {
            return m_sendTraceLoggingFn(providerMetaSize, pProviderMeta, traceLoggingMetaSize, pTraceLoggingMeta,
                traceLoggingPayloadSize, pTraceLoggingPayload, pEventDescriptor, pActivityId, pRelatedActivityId, 0);
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    ETWTelemetrySystem::ETWTelemetrySystem(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher)
        :
        TelemetrySystemBase(logManager, runtimeConfig, taskDispatcher)
    {
        //
        // Management
        //

        onStart = stats.onStart;
        onStop = stats.onStop;

#if 0 /* XXX: [MG] - don't use RouteSink for that. It's an overkill */
        this->started >> stats.onStart;
        this->stopped >> stats.onStop;
        this->paused;
        this->resumed;
#endif

        // Incoming events

        // On an arbitrary user thread
        this->sending >> stats.onIncomingEventAccepted >> this->incomingEventPrepared;

#ifndef _WINRT_DLL

        // For desktop windows, there is an API 'UtcSendTraceLogging' available for sending events over RPC.
        // This dll is available starting in Windows Vibranium and is available on desktop windows and all other windows
        // image types built on top of OneCoreUAP.

        m_diagnosticDataModule = LoadLibraryExW(L"diagnosticdataquery.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (m_diagnosticDataModule != NULL)
        {
            m_sendTraceLogging2Fn = (UtcSendTraceLogging2Fn)GetProcAddress(m_diagnosticDataModule, "UtcSendTraceLogging2");

            if (m_sendTraceLogging2Fn != nullptr)
            {
                auto config = logManager.GetLogConfiguration();
                config[CFG_STR_UTC][CFG_BOOL_UTC_LARGE_PAYLOADS] = true;
            }
            else
            {
                m_sendTraceLoggingFn = (UtcSendTraceLoggingFn)GetProcAddress(m_diagnosticDataModule, "UtcSendTraceLogging");

                if (m_sendTraceLoggingFn != nullptr)
                {
                    auto config = logManager.GetLogConfiguration();
                    config[CFG_STR_UTC][CFG_BOOL_UTC_LARGE_PAYLOADS] = true;
                }
            }
        }

#endif 

        // On the inner worker thread
        //this->preparedIncomingEvent;

       // stats.eventGenerated >>  stats.onIncomingEventAccepted;
    }

    ETWTelemetrySystem::~ETWTelemetrySystem()
    {
        if (m_diagnosticDataModule != NULL)
        {
            FreeLibrary(m_diagnosticDataModule);
            m_diagnosticDataModule = NULL;
        }
    }

    void ETWTelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
    {
        //send event to UTC here
        if (0 != sendEventToUTC(event))
        {
            DispatchEvent(DebugEventType::EVT_SEND_FAILED);
        }
    }

    int ETWTelemetrySystem::sendEventToUTC(IncomingEventContextPtr const& eventCtx)
    {
        std::string appInfoAppName = eventCtx->source->extApp[0].id;

        ProviderData providerdata;
        {
            LOCKGUARD(providerTokenLock);
            providerdata = getProviderForToken(eventCtx->record.tenantToken);
            if (0 == providerdata.providerHandle)
            { // invalid handle for provider, no need to log event for this token
                return -1;
            }
        }

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

        eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_MAKE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_MODEL);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_TIMEZONE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_LANGUAGE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SOURCE);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_ADVERTISINGID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_EXPERIMENTETAG);

        //eventCtx->source->data[0].properties.erase(SESSION_ID);
        //eventCtx->source->data[0].properties.erase(SESSION_STATE);
        //eventCtx->source->data[0].properties.erase(SESSION_DURATION);
        //eventCtx->source->data[0].properties.erase(SESSION_DURATION_BUCKET);
        //eventCtx->source->data[0].properties.erase(SESSION_FIRST_TIME);
        //eventCtx->source->data[0].properties.erase(SESSION_SDKUID);


        // PartA_Exts_CommonFields

        // Correlation Vector is passed to UTC as a regular top-level field with a reserved name.
        if (!eventCtx->source->cV.empty())
        {
            builder.AddField(CorrelationVector::PropertyName, TypeUtf8String);
            dbuilder.AddString(eventCtx->source->cV.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_EVENT_PRIVTAGS) != eventCtx->source->data[0].properties.end()) {
            // UTC mode sends privacy tag in ext.metadata.privTags
            builder.AddField(UTC_PARTA_METADATA_PRIVTAGS, TypeUInt64);
            int64_t value = eventCtx->source->data[0].properties[COMMONFIELDS_EVENT_PRIVTAGS].longValue;
            dbuilder.AddValue(value);
        }

        //PartA_Exts_CommonFields
        builder.AddField(UTC_PARTA_IKEY, TypeUtf8String);
        std::string iKey(IKEY_PRE_TEXT);
        iKey.append(eventCtx->record.tenantToken);
        dbuilder.AddString(iKey.c_str());

        if (!appInfoAppName.empty())
        {
            builder.AddField(UTC_PARTA_APP_NAME, TypeUtf8String);
            dbuilder.AddString(appInfoAppName.c_str());
        }

        std::string appInfoExpIds = eventCtx->source->extApp[0].expId;
        if (!appInfoExpIds.empty())
        {
            builder.AddField(UTC_APP_EXPERIMENT_IDS, TypeUtf8String);
            dbuilder.AddString(appInfoExpIds.c_str());
        }

        std::string deviceInfoNetworkProvider = eventCtx->source->extNet[0].provider;
        if (!deviceInfoNetworkProvider.empty())
        {
            builder.AddField(UTC_PARTA_NET_PROVIDER, TypeUtf8String);
            dbuilder.AddString(deviceInfoNetworkProvider.c_str());
        }

        std::string deviceInfoNetworkCost = eventCtx->source->extNet[0].cost;
        if (!deviceInfoNetworkCost.empty())
        {
            builder.AddField(UTC_PARTA_NET_COST, TypeUtf8String);
            dbuilder.AddString(deviceInfoNetworkCost.c_str());
        }

        std::string deviceInfoNetworkType = eventCtx->source->extNet[0].type;
        if (!deviceInfoNetworkType.empty())
        {
            builder.AddField(UTC_PARTA_NET_TYPE, TypeUtf8String);
            dbuilder.AddString(deviceInfoNetworkType.c_str());
        }

        /*
                std::string eventInfoSequence = eventCtx->source->Extension[EventInfo_Sequence];
                  eventCtx->source->Extension.erase(EventInfo_Sequence);
                  if (!eventInfoSequence.empty())
                  {
                      builder.AddField(UTC_PARTA_APP_SEQ_NUM, TypeUtf8String);
                      dbuilder.AddString(eventInfoSequence.c_str());
                  }
        */
        std::string sdkUserId(eventCtx->source->extUser[0].localId);
        if (!sdkUserId.empty())
        {
            std::string userId("e:");
            userId.append(sdkUserId);
            builder.AddField(UTC_PARTA_APP_USERID, TypeUtf8String);
            dbuilder.AddString(userId.c_str());
        }

        std::string userInfoLanguage = eventCtx->source->extUser[0].locale;
        if (!userInfoLanguage.empty())
        {
            builder.AddField(UTC_PARTA_OS_LOCALE, TypeUtf8String);
            dbuilder.AddString(userInfoLanguage.c_str());
        }

        // Rename generic viewing MetaData fields into DDV-specific names UTC understands
        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGPRODUCERID) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingProducerId", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGPRODUCERID].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGCATEGORY) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingCategory", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGCATEGORY].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingPayloadDecoderPath", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingPayloadEncodedFieldName", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGEXTRA1) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingExtra1", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGEXTRA1].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGEXTRA2) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingExtra2", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGEXTRA2].stringValue;
            dbuilder.AddString(value.c_str());
        }

        if (eventCtx->source->data[0].properties.find(COMMONFIELDS_METADATA_VIEWINGEXTRA3) != eventCtx->source->data[0].properties.end())
        {
            builder.AddField("UTCMetadata_ViewingExtra3", TypeUtf8String);
            std::string value = eventCtx->source->data[0].properties[COMMONFIELDS_METADATA_VIEWINGEXTRA3].stringValue;
            dbuilder.AddString(value.c_str());
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
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPRODUCERID);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGCATEGORY);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA1);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA2);
        eventCtx->source->data[0].properties.erase(COMMONFIELDS_METADATA_VIEWINGEXTRA3);

        //"Extension"
        PutData(eventCtx->source->ext, MD, builder, dbuilder);
        PutData(eventCtx->source->baseData, MD, builder, dbuilder);
        PutData(eventCtx->source->data, MD, builder, dbuilder);

        //PartA_Ext_ariaMD
        if (MD.size() > 0)
        {
            auto  structPartAExtsMD = builder.AddStruct(UTC_PARTA_ARIA_METADATA);
            structPartAExtsMD.AddFieldFixedArray(UTC_PARTA_ARIA_METADATA_FIELDS, static_cast<UINT16>(MD.size()), TypeUtf8String);
            std::vector<std::string>::iterator iterMD;
            for (iterMD = MD.begin(); iterMD != MD.end(); ++iterMD)
            {
                dbuilder.AddString(iterMD->c_str());
            }
        }


        if (!builder.End()) // Returns false if the metadata is too large.
        {
            return -1; // if event is too big for UTC to handle
        }

        tld::EventDescriptor eventDescriptor;// 3//level//,        opcode,        task,        keywords);     

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
        int64_t eventByteSize = byteDataVector.size() + byteVector.size()
            + providerdata.providerMetaVector.size();
        int64_t eventKBSize = (eventByteSize + 1024 - 1) / 1024;
        bool isLargeEvent = eventKBSize >= LargeEventSizeKB;

        if (!isLargeEvent)
        {
            MAT::GUID_t guid(providerdata.providerGuid);
            LOG_INFO("Sending UTC event Id=%u, provider GUID=%s", (uint16_t)(eventDescriptor.Id), guid.to_string().c_str());
            HRESULT writeResponse = tld::WriteEvent(
                providerdata.providerHandle,
                eventDescriptor,
                providerdata.providerMetaVector.data(),
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
        // if event size is larger than LargeEventSizeKB
        // or was sent to ETW and the response was STATUS_INTEGER_OVERFLOW
        if (isLargeEvent)
        {
            // Use RPC path for 64KB+ events
            if (FAILED(RPCWriteEvent(
                providerdata.providerGuid,
                (USHORT) providerdata.providerMetaVector.size(),
                providerdata.providerMetaVector.data(),
                (USHORT) byteVector.size(),
                byteVector.data(),
                (ULONG) byteDataVector.size(),
                byteDataVector.data(),
                &eventDescriptor)))
            {
                return -1;
            }
        }

        //    m_telemetryStatsHelper.UpdateOnEventsSuccessfullySentPerPriority(eventCtx->record.priority, eventCtx->source->Timestamp, 1);

        //    _GenerateAndSendStatsEventsAsNeeded(ACT_STATS_ROLLUP_KIND_ONGOING);
        //    cout << "\n_SendAriaEventToUTC  for Tenant Token = "<< eventCtx.tenantToken<< endl;
        return 0;
    }

    void ETWTelemetrySystem::PutData(std::vector< ::CsProtocol::Data>& ext,
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
                    if(!(iterValue->second.attributes.size() > 0 && iterValue->second.attributes[0].pii.size() > 0))
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

                        metadata.append(std::to_string(AriaTypeInt64));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueBool:
                    {
                        builder.AddField(name.c_str(), TypeBool8);
                        UINT8 temp = static_cast<UINT8>(iterValue->second.longValue);
                        dbuilder.AddByte(temp);

                        metadata.append(std::to_string(AriaTypeBool));
                        break;
                    }
                    case CsProtocol::ValueKind::ValueDateTime:
                    {
                        builder.AddField(name.c_str(), TypeUInt64);
                        UINT64 temp = iterValue->second.longValue;
                        dbuilder.AddBytes(&temp, sizeof(UINT64));

                        metadata.append(std::to_string(AriaTypeDateTime));
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

                        metadata.append(std::to_string(AriaTypeDouble));
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

                                metadata.append(std::to_string(AriaTypeGuid));
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
                                UINT8 ScrubType = 1; //PIIScrubber{ NotSet = 0, O365 = 1, SkypeBI = 2, SkypeData = 3} always set to O365
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

    ProviderData ETWTelemetrySystem::getProviderForToken(const std::string& tenantToken)
    {
        ProviderData temp;
        temp.providerGuid = GUID_NULL;

        if (!tenantToken.empty())
        {
            // register ikey with UTC 
            std::map<std::string, bool>::iterator iterIkey = tokenToIkeyaMap.find(tenantToken);
            if (iterIkey == tokenToIkeyaMap.end())
            {
                std::string iKey(IKEY_PRE_TEXT);
                iKey.append(tenantToken);
                int storageSize = 1024 * 8192; // min 1 MB Max 100 MB
                int uploadQuotaSize = 0;

                bool registrationSuccessful = PAL::RegisterIkeyWithWindowsTelemetry(iKey, storageSize, uploadQuotaSize);
                if (!registrationSuccessful)
                {
                    // this is a known issue inside containers. To unblock that scenario, we ignore the result of this call and
                    // assume it either succeeded or failed with the registration being done on the host side of the
                    // connection. We log here to make sure the failure isn't completely silent.
                    DispatchEvent(DebugEventType::EVT_SEND_SKIP_UTC_REGISTRATION);
                }

                tokenToIkeyaMap[tenantToken] = true;
            }
            // iKey is registered

            std::string tenantId = tenantToken.substr(0, tenantToken.find('-'));
            std::map<std::string, ProviderData>::iterator iter = tokenToProviderDataMap.find(tenantId);
            if (iter == tokenToProviderDataMap.end())
            { // register this tenant token with UTC and return provider
                bool error = false;
                temp.providerGuid = ConvertGuidStringToGUID(tenantId, error); // convert tenant token to GUID.
                if (error)
                {  // for testing with UTC or when we cann't convert guid from tannet id
                    temp.providerGuid.Data1 = 0x8E5B3D14;
                    temp.providerGuid.Data2 = 0x4D34;
                    temp.providerGuid.Data3 = 0x4029;
                    temp.providerGuid.Data4[0] = 0xB7;
                    temp.providerGuid.Data4[1] = 0x76;
                    temp.providerGuid.Data4[2] = 0xA5;
                    temp.providerGuid.Data4[3] = 0x01;
                    temp.providerGuid.Data4[4] = 0x08;
                    temp.providerGuid.Data4[5] = 0x96;
                    temp.providerGuid.Data4[6] = 0x6E;
                    temp.providerGuid.Data4[7] = 0x06;
                }

                std::string guidStr = m_config.GetProviderGroupId();
                guidStr.erase(std::remove(guidStr.begin(), guidStr.end(), '-'), guidStr.end());
                error = false;
                GUID providerGroupGuid = ConvertGuidStringToGUID(guidStr, error);
                if (error)
                {
                    // default provider group GUID to guid null
                    providerGroupGuid = GUID_NULL;
                }

                tld::ProviderMetadataBuilder<std::vector<BYTE>> providerMetaBuilder(temp.providerMetaVector);

                std::string name("Aria.");
                name.append(tenantId);

                providerMetaBuilder.Begin(name.c_str());
                providerMetaBuilder.AddTrait(ProviderTraitType::ProviderTraitGroupGuid, (void*)&providerGroupGuid, sizeof(GUID));
                providerMetaBuilder.End();

                REGHANDLE hProvider = 0;
                if (0 == RegisterProvider(&hProvider, &temp.providerGuid, temp.providerMetaVector.data()))
                {
                    temp.providerHandle = hProvider;
                    tokenToProviderDataMap[tenantId] = temp;
                }
            }
            else
            {
                return iter->second;
            }
        }
        return temp;
    }
} ARIASDK_NS_END
#endif

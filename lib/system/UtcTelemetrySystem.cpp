// Copyright (c) Microsoft. All rights reserved.

#include "CorrelationVector.hpp"
#include "UtcTelemetrySystem.hpp"
#include "LogManager.hpp"
#include "pal/UtcHelpers.hpp"
#include "utils/Utils.hpp"
#include "MicrosoftTelemetry.h"


using namespace tld;

namespace ARIASDK_NS_BEGIN {

    static const unsigned char InvalidHexDigit = 0xFF;

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

UtcTelemetrySystem::UtcTelemetrySystem(LogConfiguration& configuration, IRuntimeConfig& runtimeConfig, ContextFieldsProvider const& globalContext)
  : stats(runtimeConfig, globalContext, this),
    m_configuration(configuration)
{
    //
    // Management
    //

    this->started >> stats.onStart;

    this->stopped >> stats.onStop;

    this->paused;

    this->resumed;

    // Incoming events

    // On an arbitrary user thread
    this->addIncomingEvent >> stats.onIncomingEventAccepted  >> this->incomingEventPrepared;

    // On the inner worker thread
    //this->preparedIncomingEvent;

   // stats.eventGenerated >>  stats.onIncomingEventAccepted;
}

UtcTelemetrySystem::~UtcTelemetrySystem()
{
}

void UtcTelemetrySystem::start()
{
}

void UtcTelemetrySystem::stop()
{
}

void UtcTelemetrySystem::UploadNow()
{
}

void UtcTelemetrySystem::pauseTransmission()
{
}

void UtcTelemetrySystem::resumeTransmission()
{
}

void UtcTelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
{
    //send event to UTC here
    if (0 != sendAriaEventToUTC(event))
    {
        LogManager::DispatchEvent(DebugEventType::EVT_LOG_FAILURE); 
    }
}

void UtcTelemetrySystem::startAsync()
{
}

void UtcTelemetrySystem::stopAsync()
{
}

void UtcTelemetrySystem::handleFlushWorkerThread()
{
}

void UtcTelemetrySystem::signalDoneEvent()
{
}

void UtcTelemetrySystem::pauseTransmissionAsync()
{
}

void UtcTelemetrySystem::resumeTransmissionAsync()
{
}

void UtcTelemetrySystem::preparedIncomingEventAsync(IncomingEventContextPtr const& event)
{
    preparedIncomingEvent(event);
}

void UtcTelemetrySystem::addIncomingEventSystem(IncomingEventContextPtr const& event)
{
    addIncomingEvent(event);
}

int UtcTelemetrySystem::sendAriaEventToUTC(IncomingEventContextPtr const& eventCtx)
{
    std::string appInfoAppName = eventCtx->source->extApp[0].id;

    ProviderData providerdata = getProviderFortoken(eventCtx->record.tenantToken);
    if (0 == providerdata.providerHandle)
    { // invalid handle for provider, no need to log event for this token
        return -1;
    }

    UINT32 eventTags = MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE;//   MICROSOFT_KEYWORD_TELEMETRY  

    if (eventCtx->record.persistence > EventPersistence::EventPersistence_Normal)
    {
        eventTags = MICROSOFT_EVENTTAG_CRITICAL_PERSISTENCE;
    }

    if (eventCtx->policyBitFlags & MICROSOFT_EVENTTAG_CORE_DATA)
    {
        eventTags = eventTags | MICROSOFT_EVENTTAG_CORE_DATA;
    }

    if (eventCtx->policyBitFlags & MICROSOFT_EVENTTAG_REALTIME_LATENCY)
    {
        eventTags = eventTags | MICROSOFT_EVENTTAG_REALTIME_LATENCY;
    }
    else if (eventCtx->policyBitFlags & MICROSOFT_EVENTTAG_COSTDEFERRED_LATENCY)
    {
        eventTags = eventTags | MICROSOFT_EVENTTAG_COSTDEFERRED_LATENCY;
    }
    else
    {
        eventTags = eventTags | MICROSOFT_EVENTTAG_NORMAL_LATENCY;
    }

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
    eventCtx->source->data[0].properties.erase(EventInfo_Source);
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
        builder.AddField(CorrelationVector::PropertyName, TypeMbcsString);
        dbuilder.AddString(eventCtx->source->cV.c_str());
    }

   
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_MSAID);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_DEVICE_ID);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_NAME);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_VERSION);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_OS_BUILD);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_TIME);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_TYPE);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_OSNAME);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_SKU);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_PROJECTION);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_VER_NUM);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_EVENT_SDK_ECS);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_USER_ANID);
    eventCtx->source->data[0].properties.erase(COMMONFIELDS_APP_VERSION);
    eventCtx->source->data[0].properties.erase(EventInfo_Name);
    eventCtx->source->data[0].properties.erase(EventInfo_InitId);

    //"Extension"
    PutData(eventCtx->source->ext, MD, builder, dbuilder);
    PutData(eventCtx->source->baseData, MD, builder, dbuilder);
    PutData(eventCtx->source->data, MD, builder, dbuilder);

    //PartA_Ext_ariaMD
    if (MD.size() > 0)
    {
        auto  structPartAExtsMD = builder.AddStruct(UTC_PARTA_ARIA_METADATA);
        structPartAExtsMD.AddFieldFixedArray(UTC_PARTA_ARIA_METADATA_FIELDS, static_cast<UINT16>(MD.size()), TypeMbcsString);
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

    tld::EventDescriptor eventDescriptor;// 3//level//,		opcode,		task,		keywords);	 

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

    {
        if (0 != tld::WriteEvent(
            providerdata.providerHandle,
            eventDescriptor,
            providerdata.providerMetaVector.data(),
            byteVector.data(),
            3,
            pDataDescriptors))
        {
            return -1;
        }
    }

//	m_telemetryStatsHelper.UpdateOnEventsSuccessfullySentPerPriority(eventCtx->record.priority, eventCtx->source->Timestamp, 1);

//	_GenerateAndSendStatsEventsAsNeeded(ACT_STATS_ROLLUP_KIND_ONGOING);
//	cout << "\n_SendAriaEventToUTC  for Tenant Token = "<< eventCtx.tenantToken<< endl;
    return 0;
}

void UtcTelemetrySystem::PutData(std::vector< ::AriaProtocol::Data>& ext,
                                 std::vector<std::string>& MD,
                                 tld::EventMetadataBuilder<std::vector<BYTE>>& builder,
                                 tld::EventDataBuilder<std::vector<BYTE>>& dbuilder )
{
    if (ext.size() > 0)
    {
        std::vector<::AriaProtocol::Data>::const_iterator iterExtension;
        for (iterExtension = ext.begin(); iterExtension != ext.end(); ++iterExtension)
        {
            std::map<std::string, AriaProtocol::Value>::const_iterator iterValue;
            for (iterValue = iterExtension->properties.begin(); iterValue != iterExtension->properties.end(); ++iterValue)
            {
                std::string name = iterValue->first;
                std::string metadata(ARIA_METADATA_COMMON_TEXT);
                metadata.append(name.c_str());
                metadata.append(ARIA_METADATA_TYPE_TEXT);
                bool addMetaData = true;
                bool error = false;

                switch (iterValue->second.type)
                {
                case AriaProtocol::ValueKind::ValueInt64:
                case AriaProtocol::ValueKind::ValueUInt64:
                case AriaProtocol::ValueKind::ValueInt32:
                case AriaProtocol::ValueKind::ValueUInt32:
                {
                    builder.AddField(name.c_str(), TypeInt64);
                    INT64 temp = iterValue->second.longValue;
                    dbuilder.AddBytes(&temp, sizeof(INT64));

                    metadata.append(std::to_string(AriaTypeInt64));
                    break;
                }
                case AriaProtocol::ValueKind::ValueBool:
                {
                    builder.AddField(name.c_str(), TypeBool8);
                    UINT8 temp = static_cast<UINT8>(iterValue->second.longValue);
                    dbuilder.AddByte(temp);

                    metadata.append(std::to_string(AriaTypeBool));
                    break;
                }
                case AriaProtocol::ValueKind::ValueDateTime:
                {
                    builder.AddField(name.c_str(), TypeUInt64);
                    UINT64 temp = iterValue->second.longValue;
                    dbuilder.AddBytes(&temp, sizeof(UINT64));

                    metadata.append(std::to_string(AriaTypeDateTime));
                    break;
                }
                case AriaProtocol::ValueKind::ValueArrayInt64:
                case AriaProtocol::ValueKind::ValueArrayUInt64:
                case AriaProtocol::ValueKind::ValueArrayInt32:
                case AriaProtocol::ValueKind::ValueArrayUInt32:
                case AriaProtocol::ValueKind::ValueArrayBool:
                case AriaProtocol::ValueKind::ValueArrayDateTime:
                {

                    break;
                }
                case AriaProtocol::ValueKind::ValueDouble:
                {
                    builder.AddField(name.c_str(), TypeDouble);
                    double temp = iterValue->second.doubleValue;
                    dbuilder.AddBytes(&temp, sizeof(double));

                    metadata.append(std::to_string(AriaTypeDouble));
                    break;
                }
                case AriaProtocol::ValueKind::ValueArrayDouble:
                {
                    break;
                }
                case AriaProtocol::ValueKind::ValueString:
                {
                    addMetaData = false;
                    builder.AddField(name.c_str(), TypeMbcsString);
                    dbuilder.AddString(iterValue->second.stringValue.c_str());
                    break;
                }
                case AriaProtocol::ValueKind::ValueArrayString:
                {
                    addMetaData = false;
                    builder.AddField(name.c_str(), TypeMbcsString);
                    dbuilder.AddString(iterValue->second.stringValue.c_str());
                    break;
                }
                case AriaProtocol::ValueKind::ValueGuid:
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
                case AriaProtocol::ValueKind::ValueArrayGuid:
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

ProviderData UtcTelemetrySystem::getProviderFortoken(const std::string& tenantToken)
{
    ProviderData  temp;
    temp.providerHandle = 0;

    if (!tenantToken.empty())
    {
        //register ikey with UTC 
        std::map<std::string, bool>::iterator iterIkey = tokenToIkeyaMap.find(tenantToken);
        if (iterIkey == tokenToIkeyaMap.end())
        {
            std::string iKey(IKEY_PRE_TEXT);
            iKey.append(tenantToken);
            int StorageSize = 1024 * 8192; //min 1 MB Max 100 MB
            int UploadQuotaSize = 0;
            if (PAL::RegisterIkeyWithWindowsTelemetry(iKey, StorageSize, UploadQuotaSize))
            {
                tokenToIkeyaMap[tenantToken] = true;
            }
			else
			{
				return temp; // Ikey registeration failed, we ignore this event
			}
        }
        //iKey is registered


        std::string tenantId = tenantToken.substr(0, tenantToken.find('-')); 
        std::map<std::string, ProviderData>::iterator iter = tokenToProviderDataMap.find(tenantId);
        if (iter == tokenToProviderDataMap.end())
        {// register this tenant token with UTC and return provider
            bool error = false;
            GUID myGuid = ConvertGuidStringToGUID(tenantId, error); //convert tenant token to GUID.
            if (error)
            {  // for testing with UTC or when we cann't convert guid from tannet id
                myGuid.Data1 = 0x8E5B3D14;
                myGuid.Data2 = 0x4D34;
                myGuid.Data3 = 0x4029;
                myGuid.Data4[0] = 0xB7;
                myGuid.Data4[1] = 0x76;
                myGuid.Data4[2] = 0xA5;
                myGuid.Data4[3] = 0x01;
                myGuid.Data4[4] = 0x08;
                myGuid.Data4[5] = 0x96;
                myGuid.Data4[6] = 0x6E;
                myGuid.Data4[7] = 0x06;
            }

            //aria Group Guid   780dddc8-18a1-5781-895a-a690464fa89c telemetryt group Guid 4f50731a - 89cf - 4782 - b3e0 - dce8c90476ba
            GUID myGroupGuid;
            myGroupGuid.Data1 = 0x780dddc8;
            myGroupGuid.Data2 = 0x18a1;
            myGroupGuid.Data3 = 0x5781;
            myGroupGuid.Data4[0] = 0x89;
            myGroupGuid.Data4[1] = 0x5a;
            myGroupGuid.Data4[2] = 0xa6;
            myGroupGuid.Data4[3] = 0x90;
            myGroupGuid.Data4[4] = 0x46;
            myGroupGuid.Data4[5] = 0x4f;
            myGroupGuid.Data4[6] = 0xa8;
            myGroupGuid.Data4[7] = 0x9c;

            tld::ProviderMetadataBuilder<std::vector<BYTE>> providerMetaBuilder(temp.providerMetaVector);

            std::string name("Aria.");// = tenantId;// providerName;
                                      //name.append("-");
            name.append(tenantId);

            providerMetaBuilder.Begin(name.c_str());
            providerMetaBuilder.AddTrait(ProviderTraitType::ProviderTraitGroupGuid, (void*)&myGroupGuid, sizeof(GUID));
            providerMetaBuilder.End();

            REGHANDLE hProvider;
            if (0 == RegisterProvider(&hProvider, &myGuid, temp.providerMetaVector.data()))
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

// Copyright (c) Microsoft. All rights reserved.

#include "UtcTelemetrySystem.hpp"
#include "LogManager.hpp"
#include "pal/UtcHelpers.hpp"
#include "utils/Utils.hpp"
#include "MicrosoftTelemetry.h"
#include "traceloggingdynamic.h"

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

UtcTelemetrySystem::UtcTelemetrySystem(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig, ContextFieldsProvider const& globalContext)
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
    std::string appInfoAppName = eventCtx->source->Extension[COMMONFIELDS_APP_ID];
    eventCtx->source->Extension.erase(COMMONFIELDS_APP_ID);

    ProviderData providerdata = getProviderFortoken(eventCtx->record.tenantToken);
    if (0 == providerdata.providerHandle)
    { // invalid handle for provider, no need to log event for this token
        return -1;
    }

    UINT32 eventTags = MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE;//   MICROSOFT_KEYWORD_TELEMETRY  

    if (eventCtx->record.priority > EventPriority::EventPriority_Normal)
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

    
    std::string eventName = eventCtx->source->Extension[EventInfo_Name];
    if (eventName.empty())
    {
        eventName = eventCtx->source->EventType;
    }

    builder.Begin(eventName.c_str(), eventTags);

    if (m_configuration.sdkmode == SdkModeTypes::SdkModeTypes_UTCAriaBackCompat)
    {
        //PartA_Exts_bc
        auto  structPartAExtsBC = builder.AddStruct(UTC_PART_A_EXTS_BC);

        if (!eventCtx->source->Type.empty())
        {
            structPartAExtsBC.AddField(UTC_TYPE, TypeMbcsString);
            dbuilder.AddString(eventCtx->source->Type.c_str());
        }

        std::string deviceInfoMake = eventCtx->source->Extension[COMMONFIELDS_DEVICE_MAKE];
        if (!deviceInfoMake.empty())
        {
            structPartAExtsBC.AddField(UTC_MAKE, TypeMbcsString);
            dbuilder.AddString(deviceInfoMake.c_str());
        }

        std::string deviceInfoModel = eventCtx->source->Extension[COMMONFIELDS_DEVICE_MODEL];
        if (!deviceInfoModel.empty())
        {
            structPartAExtsBC.AddField(UTC_MODEL, TypeMbcsString);
            dbuilder.AddString(deviceInfoModel.c_str());
        }

        std::string userInforTimeZone = eventCtx->source->Extension[COMMONFIELDS_USER_TIMEZONE];
        if (!userInforTimeZone.empty())
        {
            structPartAExtsBC.AddField(UTC_TIMEZONE, TypeMbcsString);
            dbuilder.AddString(userInforTimeZone.c_str());
        }

        std::string appInfoLanguage = eventCtx->source->Extension[COMMONFIELDS_APP_LANGUAGE];
        if (!appInfoLanguage.empty())
        {
            structPartAExtsBC.AddField(UTC_APP_LANG, TypeMbcsString);
            dbuilder.AddString(appInfoLanguage.c_str());
        }

        std::string eventSource = eventCtx->source->Extension[EventInfo_Source];
        if (!eventSource.empty())
        {
            structPartAExtsBC.AddField(UTC_BC_SOURCE, TypeMbcsString);
            dbuilder.AddString(eventSource.c_str());
        }

        std::string userInfoAdvertisingId(eventCtx->source->Extension[COMMONFIELDS_USER_ADVERTISINGID]);
        if (!userInfoAdvertisingId.empty())
        {
            structPartAExtsBC.AddField(UTC_BC_ADVERTISEMENT_ID, TypeMbcsString);
            dbuilder.AddString(userInfoAdvertisingId.c_str());
        }
    }

    eventCtx->source->Extension.erase(COMMONFIELDS_DEVICE_MAKE);
    eventCtx->source->Extension.erase(COMMONFIELDS_DEVICE_MODEL);
    eventCtx->source->Extension.erase(COMMONFIELDS_USER_TIMEZONE);
    eventCtx->source->Extension.erase(COMMONFIELDS_APP_LANGUAGE);
    eventCtx->source->Extension.erase(EventInfo_Source);
    eventCtx->source->Extension.erase(COMMONFIELDS_USER_ADVERTISINGID);

    //PartA_Exts_aria
    auto  structPartAExtsAria = builder.AddStruct(UTC_PART_A_EXTS_ARIA);
    structPartAExtsAria.AddField(UTC_LIB_VERSION, TypeMbcsString);
    std::string eventInfoSdkVersion = eventCtx->source->Extension[COMMONFIELDS_EVENT_SDKVERSION];
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDKVERSION);
    dbuilder.AddString(eventInfoSdkVersion.c_str());

    if (m_configuration.sdkmode == SdkModeTypes::SdkModeTypes_UTCAriaBackCompat)
    {
        std::string appInfoEtag = eventCtx->source->Extension[COMMONFIELDS_APP_EXPERIMENTETAG];
        if (!appInfoEtag.empty())
        {
            structPartAExtsAria.AddField(UTC_APP_ETAG, TypeMbcsString);
            dbuilder.AddString(appInfoEtag.c_str());
        }

        std::string eventsessiond = eventCtx->source->Extension[SESSION_ID];
        if (!eventsessiond.empty())
        {
            structPartAExtsAria.AddField(UTC_PARTA_APP_SESSION_ID, TypeMbcsString);
            dbuilder.AddString(eventsessiond.c_str());
        }

        structPartAExtsAria.AddField(UTC_PARTA_RECORD_TIMESTAMP, TypeInt64);
        INT64 temp = eventCtx->source->Timestamp;
        dbuilder.AddValue(temp);
    }

    eventCtx->source->Extension.erase(COMMONFIELDS_APP_EXPERIMENTETAG);
    eventCtx->source->Extension.erase(SESSION_ID);
    //eventCtx->source->Extension.erase(SESSION_STATE);
    //eventCtx->source->Extension.erase(SESSION_DURATION);
    //eventCtx->source->Extension.erase(SESSION_DURATION_BUCKET);
    //eventCtx->source->Extension.erase(SESSION_FIRST_TIME);
    //eventCtx->source->Extension.erase(SESSION_SDKUID);


    //PartA_Exts_CommonFields
    builder.AddField(UTC_PARTA_IKEY, TypeMbcsString);
    std::string iKey(IKEY_PRE_TEXT);
    iKey.append(eventCtx->record.tenantToken);
    dbuilder.AddString(iKey.c_str());

    if (!appInfoAppName.empty())
    {
        builder.AddField(UTC_PARTA_APP_NAME, TypeMbcsString);
        dbuilder.AddString(appInfoAppName.c_str());
    }

    std::string appInfoExpIds = eventCtx->source->Extension[COMMONFIELDS_APP_EXPERIMENTIDS];
    eventCtx->source->Extension.erase(COMMONFIELDS_APP_EXPERIMENTIDS);
    if (!appInfoExpIds.empty())
    {
        builder.AddField(UTC_APP_EXPERIMENT_IDS, TypeMbcsString);
        dbuilder.AddString(appInfoExpIds.c_str());
    }

    std::string deviceInfoNetworkProvider = eventCtx->source->Extension[COMMONFIELDS_NETWORK_PROVIDER];
    eventCtx->source->Extension.erase(COMMONFIELDS_NETWORK_PROVIDER);
    if (!deviceInfoNetworkProvider.empty())
    {
        builder.AddField(UTC_PARTA_NET_PROVIDER, TypeMbcsString);
        dbuilder.AddString(deviceInfoNetworkProvider.c_str());
    }

    std::string deviceInfoNetworkCost = eventCtx->source->Extension[COMMONFIELDS_NETWORK_COST];
    eventCtx->source->Extension.erase(COMMONFIELDS_NETWORK_COST);
    if (!deviceInfoNetworkCost.empty())
    {
        builder.AddField(UTC_PARTA_NET_COST, TypeMbcsString);
        dbuilder.AddString(deviceInfoNetworkCost.c_str());
    }

    std::string deviceInfoNetworkType = eventCtx->source->Extension[COMMONFIELDS_NETWORK_TYPE];
    eventCtx->source->Extension.erase(COMMONFIELDS_NETWORK_TYPE);
    if (!deviceInfoNetworkType.empty())
    {
        builder.AddField(UTC_PARTA_NET_TYPE, TypeMbcsString);
        dbuilder.AddString(deviceInfoNetworkType.c_str());
    }

    std::string eventInfoSequence = eventCtx->source->Extension[EventInfo_Sequence];
    eventCtx->source->Extension.erase(EventInfo_Sequence);
    if (!eventInfoSequence.empty())
    {
        builder.AddField(UTC_PARTA_APP_SEQ_NUM, TypeMbcsString);
        dbuilder.AddString(eventInfoSequence.c_str());
    }

    std::string ariaUserId(eventCtx->source->Extension[COMMONFIELDS_USER_ID]);
    eventCtx->source->Extension.erase(COMMONFIELDS_USER_ID);
    if (!ariaUserId.empty())
    {
        std::string userId("e:");
        userId.append(ariaUserId);
        builder.AddField(UTC_PARTA_APP_USERID, TypeMbcsString);
        dbuilder.AddString(userId.c_str());
    }

    std::string userInfoLanguage = eventCtx->source->Extension[COMMONFIELDS_USER_LANGUAGE];
    eventCtx->source->Extension.erase(COMMONFIELDS_USER_LANGUAGE);
    if (!userInfoLanguage.empty())
    {
        builder.AddField(UTC_PARTA_OS_LOCALE, TypeMbcsString);
        dbuilder.AddString(userInfoLanguage.c_str());
    }

    eventCtx->source->Extension.erase(COMMONFIELDS_USER_MSAID);
    eventCtx->source->Extension.erase(COMMONFIELDS_DEVICE_ID);
    eventCtx->source->Extension.erase(COMMONFIELDS_OS_NAME);
    eventCtx->source->Extension.erase(COMMONFIELDS_OS_VERSION);
    eventCtx->source->Extension.erase(COMMONFIELDS_OS_BUILD);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_TIME);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_TYPE);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_OSNAME);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_SKU);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_PROJECTION);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_VER_NUM);
    eventCtx->source->Extension.erase(COMMONFIELDS_EVENT_SDK_ECS);
    eventCtx->source->Extension.erase(COMMONFIELDS_USER_ANID);
    eventCtx->source->Extension.erase(COMMONFIELDS_APP_VERSION);
    eventCtx->source->Extension.erase(EventInfo_Name);
    eventCtx->source->Extension.erase(EventInfo_InitId);

    //"ConfigurationIds"
    if (eventCtx->source->ConfigurationIds.size() > 0)
    {
        std::map<std::string, std::string>::iterator iterConfigId;
        for (iterConfigId = eventCtx->source->ConfigurationIds.begin(); iterConfigId != eventCtx->source->ConfigurationIds.end(); ++iterConfigId)
        {
            if (!iterConfigId->second.empty())
            {
                std::string name = iterConfigId->first;

                builder.AddField(name.c_str(), TypeMbcsString);
                dbuilder.AddString(iterConfigId->second.c_str());
            }
        }
    }

    //"Extension"
    if (eventCtx->source->Extension.size() > 0)
    {
        std::map<std::string, std::string>::iterator iterExtension;
        for (iterExtension = eventCtx->source->Extension.begin(); iterExtension != eventCtx->source->Extension.end(); ++iterExtension)
        {
            if (!iterExtension->second.empty())
            {
                std::string name = iterExtension->first;

                builder.AddField(name.c_str(), TypeMbcsString);
                dbuilder.AddString(iterExtension->second.c_str());
            }
        }
    }

    //"ContextIds"
    if (eventCtx->source->ContextIds.size() > 0)
    {
        std::map<std::string, std::string>::iterator iterContextIds;
        for (iterContextIds = eventCtx->source->ContextIds.begin(); iterContextIds != eventCtx->source->ContextIds.end(); ++iterContextIds)
        {
            if (!iterContextIds->second.empty())
            {
                std::string name = iterContextIds->first;

                builder.AddField(name.c_str(), TypeMbcsString);
                dbuilder.AddString(iterContextIds->second.c_str());
            }
        }
    }

    //"PIIExtensions"
    if (eventCtx->source->PIIExtensions.size() > 0)
    {
        std::map<std::string, AriaProtocol::PII>::iterator iterPii;
        for (iterPii = eventCtx->source->PIIExtensions.begin(); iterPii != eventCtx->source->PIIExtensions.end(); ++iterPii)
        {
            if (!iterPii->second.RawContent.empty())
            {
                std::string name = iterPii->first;

                builder.AddField(name.c_str(), TypeMbcsString);
                dbuilder.AddString(iterPii->second.RawContent.c_str());

                std::string metadata(ARIA_METADATA_COMMON_TEXT);
                metadata.append(name.c_str());

                UINT8 ScrubType = static_cast<UINT8>(iterPii->second.ScrubType);
                metadata.append(";s:");
                metadata.append(std::to_string(ScrubType));

                UINT8 Kind = static_cast<UINT8>(iterPii->second.Kind);
                metadata.append(";k:");
                metadata.append(std::to_string(Kind));

                MD.push_back(metadata);
            }
        }
    }

    //TypedExtensionBoolean
    if (eventCtx->source->TypedExtensionBoolean.size() > 0)
    {
        std::map<std::string, bool>::iterator iterExtensionBooleanMap;
        for (iterExtensionBooleanMap = eventCtx->source->TypedExtensionBoolean.begin(); iterExtensionBooleanMap != eventCtx->source->TypedExtensionBoolean.end(); ++iterExtensionBooleanMap)
        {
            std::string name = iterExtensionBooleanMap->first;

            builder.AddField(name.c_str(), TypeBool8);
            dbuilder.AddByte(iterExtensionBooleanMap->second);

            std::string metadata(ARIA_METADATA_COMMON_TEXT);
            metadata.append(name.c_str());
            metadata.append(ARIA_METADATA_TYPE_TEXT);
            metadata.append(std::to_string(AriaTypeBool));
            MD.push_back(metadata);
        }
    }

    //TypedExtensionDateTime
    if (eventCtx->source->TypedExtensionDateTime.size() > 0)
    {
        std::map<std::string, INT64>::iterator iterExtensionDateTimeMap;
        for (iterExtensionDateTimeMap = eventCtx->source->TypedExtensionDateTime.begin(); iterExtensionDateTimeMap != eventCtx->source->TypedExtensionDateTime.end(); ++iterExtensionDateTimeMap)
        {
            UINT64 temp = iterExtensionDateTimeMap->second;
            if (temp > 0)
            {
                std::string name = iterExtensionDateTimeMap->first;

                builder.AddField(name.c_str(), TypeUInt64);

                dbuilder.AddBytes(&temp, sizeof(UINT64));

                std::string metadata(ARIA_METADATA_COMMON_TEXT);
                metadata.append(name.c_str());
                metadata.append(ARIA_METADATA_TYPE_TEXT);
                metadata.append(std::to_string(AriaTypeDateTime));
                MD.push_back(metadata);
            }
        }
    }

    //TypedExtensionInt64
    if (eventCtx->source->TypedExtensionInt64.size() > 0)
    {
        std::map<std::string, INT64>::iterator iterExtensionInt64Map;
        for (iterExtensionInt64Map = eventCtx->source->TypedExtensionInt64.begin(); iterExtensionInt64Map != eventCtx->source->TypedExtensionInt64.end(); ++iterExtensionInt64Map)
        {
            std::string name = iterExtensionInt64Map->first;

            builder.AddField(name.c_str(), TypeInt64);
            INT64 temp = iterExtensionInt64Map->second;
            dbuilder.AddBytes(&temp, sizeof(INT64));

            std::string metadata(ARIA_METADATA_COMMON_TEXT);
            metadata.append(name.c_str());
            metadata.append(ARIA_METADATA_TYPE_TEXT);
            metadata.append(std::to_string(AriaTypeInt64));
            MD.push_back(metadata);
        }
    }

    //TypedExtensionDouble
    if (eventCtx->source->TypedExtensionDouble.size() > 0)
    {
        std::map<std::string, double>::iterator iterExtensionDoubleMap;
        for (iterExtensionDoubleMap = eventCtx->source->TypedExtensionDouble.begin(); iterExtensionDoubleMap != eventCtx->source->TypedExtensionDouble.end(); ++iterExtensionDoubleMap)
        {
            std::string name = iterExtensionDoubleMap->first;

            builder.AddField(name.c_str(), TypeDouble);
            double temp = iterExtensionDoubleMap->second;
            dbuilder.AddBytes(&temp, sizeof(double));

            std::string metadata(ARIA_METADATA_COMMON_TEXT);
            metadata.append(name.c_str());
            metadata.append(ARIA_METADATA_TYPE_TEXT);
            metadata.append(std::to_string(AriaTypeDouble));
            MD.push_back(metadata);
        }
    }

    //TypedExtensionGuid
    if (eventCtx->source->TypedExtensionGuid.size() > 0)
    {
        std::map<std::string, std::vector<uint8_t>>::iterator iterExtensionGuidMap;
        for (iterExtensionGuidMap = eventCtx->source->TypedExtensionGuid.begin(); iterExtensionGuidMap != eventCtx->source->TypedExtensionGuid.end(); ++iterExtensionGuidMap)
        {
            bool error = false;
            GUID temp = GUID_t::convertUintVectorToGUID(iterExtensionGuidMap->second);
            if (false == error)
            {
                std::string name = iterExtensionGuidMap->first;

                builder.AddField(name.c_str(), TypeGuid);
                dbuilder.AddBytes(&temp, sizeof(GUID));

                std::string metadata(ARIA_METADATA_COMMON_TEXT);
                metadata.append(name.c_str());
                metadata.append(ARIA_METADATA_TYPE_TEXT);
                metadata.append(std::to_string(AriaTypeGuid));
                MD.push_back(metadata);
            }
        }
    }

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

    tld::EventDescriptor eventDescriptor;// 3/*level*/,		opcode,		task,		keywords);	 

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

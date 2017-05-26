// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "ITelemetrySystem.hpp"
#include <aria/Version.hpp>
#include "bond/BondSerializer.hpp"
#include "compression/HttpDeflateCompression.hpp"
#include <aria/LogConfiguration.hpp>
#include "system/Contexts.hpp"
#include "stats/Statistics.hpp"
#if ARIASDK_UTC_ENABLED
    #include "utc/UtcForwarder.hpp"
#endif

namespace ARIASDK_NS_BEGIN {

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
    const char* const UTC_PARTA_IKEY = "PartA_iKey";
    const char* const UTC_PARTA_APP_NAME = "PartA_Ext_App_Name";
    const char* const UTC_PARTA_NET_PROVIDER = "PartA_Ext_Net_Provider";
    const char* const UTC_PARTA_NET_COST = "PartA_Ext_Net_Cost";
    const char* const UTC_PARTA_NET_TYPE = "PartA_Ext_Net_Type";
    const char* const UTC_PARTA_APP_SEQ_NUM = "PartA_Ext_App_SeqNum";
    const char* const UTC_PARTA_APP_SESSION_ID = "sesId";
    const char* const UTC_PARTA_RECORD_TIMESTAMP = "evtTime";
    const char* const UTC_PARTA_APP_ASID = "PartA_Ext_App_AsId";
    const char* const UTC_PARTA_APP_USERID = "PartA_Ext_App_UserId";
    const char* const UTC_PARTA_OS_LOCALE = "PartA_Ext_Os_Locale";
    const char* const UTC_PARTA_USER_AUTH_ID = "PartA_Ext_User_AuthId";
    const char* const UTC_PARTA_ARIA_METADATA = "PartA_Ext_AriaMD";
    const char* const UTC_PARTA_ARIA_METADATA_FIELDS = "fields";
    const char* const UTC_STATS_TOKEN = "a6fb6dce35104339aec4b7c9e6a5ad46-c141d578-a12a-460d-8de3-a5b2ff0235ae-7076";

    enum AriaType : UINT16
    {
        AriaTypeString = 0,  // (Default, not emitted in “t” field)
        AriaTypeBool = 1,  // 32 bit
        AriaTypeInt64 = 2,  // 64 bit
        AriaTypeDouble = 3,  // double
        AriaTypeDateTime = 4,  // 64 bit
        AriaTypeGuid = 5   // 128 bit
    };


    typedef struct providerdata
    {
        ULONGLONG providerHandle;
        std::vector<BYTE> providerMetaVector;
    }ProviderData;

class UtcTelemetrySystem : public PAL::RefCountedImpl<UtcTelemetrySystem>,
                           public ITelemetrySystem
{
  public:
    UtcTelemetrySystem(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig, ContextFieldsProvider const& globalContext);
    ~UtcTelemetrySystem();

  public:
    void start();
    void stop();
    void pauseTransmission();
    void resumeTransmission();
    void UploadNow();
    void addIncomingEventSystem(IncomingEventContextPtr const& event);

  protected:
    void startAsync();
    void stopAsync();
    void handleFlushWorkerThread();
    void signalDoneEvent();
    void pauseTransmissionAsync();
    void resumeTransmissionAsync();
    void handleIncomingEventPrepared(IncomingEventContextPtr const& event);
    void preparedIncomingEventAsync(IncomingEventContextPtr const& event);
    int sendAriaEventToUTC(IncomingEventContextPtr const& eventCtx);
    ProviderData getProviderFortoken(const std::string& tenantToken);

  protected:
    bool                      m_isPaused;
    PAL::Event                m_doneEvent;

    BondSerializer            bondSerializer;
    Statistics                stats;
    LogConfiguration          m_configuration;
    std::mutex                m_lock;
    std::map<std::string, ProviderData> tokenToProviderDataMap;
    std::map<std::string, bool> tokenToIkeyaMap;
    bool                        m_isInitialized;

    


public:
    RouteSource<>                                              started;
    RouteSource<>                                              stopped;
    RouteSource<>                                              paused;
    RouteSource<>                                              resumed;

    RouteSource<IncomingEventContextPtr const&>                addIncomingEvent;
    RouteSink<UtcTelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{this, &UtcTelemetrySystem::handleIncomingEventPrepared};

    RouteSource<IncomingEventContextPtr const&>                preparedIncomingEvent;
};


} ARIASDK_NS_END

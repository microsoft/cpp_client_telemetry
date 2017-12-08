// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Logger.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "system/Contexts.hpp"
#include <IHttpClient.hpp>
#include <api/ILogManagerInternal.hpp>
#include <api/LogConfiguration.hpp>
#include <DebugEvents.hpp>
#include <memory>

namespace ARIASDK_NS_BEGIN {


class ITelemetrySystem;


class LogManagerImpl : public ILogManagerInternal {
  public:
    LogManagerImpl(LogConfiguration configuration, IRuntimeConfig* runtimeConfig);
    virtual ~LogManagerImpl() override;
    virtual void FlushAndTeardown() override;
    virtual void Flush() override;
	virtual void UploadNow() override;
    virtual void PauseTransmission() override;
    virtual void ResumeTransmission() override;
	virtual bool SetTransmitProfile(const std::string& profile) override;
	virtual bool LoadTransmitProfiles(const std::string& profiles_json) override;
	virtual void ResetTransmitProfiles();
	virtual void  SetTransmitProfile(TransmitProfile profile) override;
	virtual const std::string& GetTransmitProfileName() override;
	virtual ISemanticContext& GetSemanticContext() override;
	virtual void SetContext(std::string const& name, std::string const& value, PiiKind piiKind = PiiKind_None) override;
	virtual void SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) override;
	virtual void SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) override;
	virtual inline void SetContext(const std::string& name, const char *value, PiiKind piiKind = PiiKind_None) override { const std::string val(value); SetContext(name, val, piiKind); };
	virtual inline void SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual inline void SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void  SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) override;
	virtual void  SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) override;
	virtual void  SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) override;
    virtual ILogger* GetLogger(std::string const& tenantToken, ContextFieldsProvider* parentContext, std::string const& source = std::string(), std::string const& experimentationProject = std::string()) override;

    virtual void addIncomingEvent(IncomingEventContextPtr const& event) override;

  protected:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();

    std::mutex                             m_lock;
    //std::map<std::string, Logger*>         m_loggers;
    std::unique_ptr<ContextFieldsProvider> m_context;

    IHttpClient*                           m_httpClient;
    std::unique_ptr<IHttpClient>           m_ownHttpClient;

    IRuntimeConfig*                        m_runtimeConfig;
    RuntimeConfig_Default                  m_defaultRuntimeConfig;
    std::unique_ptr<IRuntimeConfig>        m_ownRuntimeConfig;

    IBandwidthController*                  m_bandwidthController;
    std::unique_ptr<IBandwidthController>  m_ownBandwidthController;

    std::unique_ptr<IOfflineStorage>       m_offlineStorage;

    ITelemetrySystem*                      m_system;

    bool                                   m_alive;
    LogConfiguration                       m_logConfiguration;
};


} ARIASDK_NS_END

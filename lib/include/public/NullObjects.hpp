//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_NULLOBJECTS_HPP
#define MAT_NULLOBJECTS_HPP

#include "ILogManager.hpp"
#include "ILogger.hpp"

namespace MAT_NS_BEGIN
{

    class NullContext : public ISemanticContext
    {
    public:

        virtual void SetNetworkCost(NetworkCost /*networkCost*/) override {};

        virtual void SetNetworkType(NetworkType /*networkType*/) override {};

        virtual void SetUserId(const std::string & /*userId*/, PiiKind /*piiKind*/ = PiiKind_Identity) override { };

        virtual void SetTicket(TicketType, const std::string &) override {};

        virtual void SetCommonField(const std::string &, const EventProperty &) override {};

        virtual void SetCustomField(const std::string &, const EventProperty &) override {};
    };

    class NullEventFilterCollection : public IEventFilterCollection
    {
        virtual void RegisterEventFilter(std::unique_ptr<IEventFilter>&&) override { }
        virtual void UnregisterEventFilter(const char*) override { }
        virtual void UnregisterAllFilters() noexcept override { }
        virtual bool CanEventPropertiesBeSent(const EventProperties&) const noexcept override { return true; }
        virtual size_t Size() const noexcept override { return 0; };
        virtual bool Empty() const noexcept override { return false; };
    };

    class NullLogger : public ILogger
    {

    public:

        // Inherited via ILogger
        virtual ISemanticContext * GetSemanticContext() const override
        {
            static NullContext nullContext;
            return &nullContext;
        }

        virtual void SetContext(const std::string & /*name*/, const char /*value*/[], PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, const std::string & /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, double /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, int8_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, int16_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, int32_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, int64_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, uint8_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, uint16_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, uint32_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, uint64_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, bool /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, time_ticks_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, GUID_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override {};

        virtual void SetContext(const std::string & /*name*/, const EventProperty & /*prop*/) override {};

        virtual void LogAppLifecycle(AppLifecycleState /*state*/, EventProperties const & /*properties*/) override {};

        virtual void LogSession(SessionState /*state*/, const EventProperties & /*properties*/) override {};

        virtual void LogEvent(std::string const & /*name*/) override {};

        virtual void LogEvent(EventProperties const & /*properties*/) override {};

        virtual void LogFailure(std::string const & /*signature*/, std::string const & /*detail*/, EventProperties const & /*properties*/) override {};

        virtual void LogFailure(std::string const & /*signature*/, std::string const & /*detail*/, std::string const & /*category*/, std::string const & /*id*/, EventProperties const & /*properties*/) override {};

        virtual void LogPageView(std::string const & /*id*/, std::string const & /*pageName*/, EventProperties const & /*properties*/) override {};

        virtual void LogPageView(std::string const & /*id*/, std::string const & /*pageName*/, std::string const & /*category*/, std::string const & /*uri*/, std::string const & /*referrerUri*/, EventProperties const & /*properties*/) override {};

        virtual void LogPageAction(std::string const & /*pageViewId*/, ActionType /*actionType*/, EventProperties const & /*properties*/) override {};

        virtual void LogPageAction(PageActionData const & /*pageActionData*/, EventProperties const & /*properties*/) override {};

        virtual void LogSampledMetric(std::string const & /*name*/, double /*value*/, std::string const & /*units*/, EventProperties const & /*properties*/) override {};

        virtual void LogSampledMetric(std::string const & /*name*/, double /*value*/, std::string const & /*units*/, std::string const & /*instanceName*/, std::string const & /*objectClass*/, std::string const & /*objectId*/, EventProperties const & /*properties*/) override {};

        virtual void LogAggregatedMetric(std::string const & /*name*/, long /*duration*/, long /*count*/, EventProperties const & /*properties*/) override {};

        virtual void LogAggregatedMetric(AggregatedMetricData const & /*metricData*/, EventProperties const & /*properties*/) override {};

        virtual void LogTrace(TraceLevel /*level*/, std::string const & /*message*/, EventProperties const & /*properties*/) override {};

        virtual void LogUserState(UserState /*state*/, long /*timeToLiveInMillis*/, EventProperties const & /*properties*/) override {};

        virtual IEventFilterCollection& GetEventFilters() noexcept override { return m_filters; }

        virtual IEventFilterCollection const& GetEventFilters() const noexcept override { return m_filters; }

        virtual void SetParentContext(ISemanticContext * /*context*/) override {};

        virtual void SetLevel(uint8_t /*level*/) override {};

    private:
        NullEventFilterCollection m_filters;
    };

    class NullDataViewerCollection : public IDataViewerCollection
    {
    public:
        virtual void DispatchDataViewerEvent(const std::vector<uint8_t>&) const noexcept override {};

        virtual void RegisterViewer(const std::shared_ptr<IDataViewer>&) override {};

        virtual void UnregisterViewer(const char*) override {};

        virtual void UnregisterAllViewers() override {};

        virtual bool IsViewerEnabled(const char*) const override
        {
            return false;
        }

        virtual bool IsViewerEnabled() const noexcept override
        {
            return false;
        }

        virtual bool IsViewerRegistered(const char*) const override
        {
            return false;
        }

        virtual ~NullDataViewerCollection() {};
    };

    class NullLogManager : public ILogManager
    {
    public:

        NullLogManager() { };

        // Inherited via ILogManager
        virtual bool DispatchEvent(DebugEvent /*evt*/) override
        {
            return false;
        }

        virtual void Configure() override {};

        virtual void FlushAndTeardown() override {};

        virtual status_t Flush() override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t UploadNow() override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t PauseTransmission() override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t ResumeTransmission() override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetTransmitProfile(TransmitProfile profile) override
        {
            UNREFERENCED_PARAMETER(profile);
            return STATUS_ENOSYS;
        }

        virtual status_t SetTransmitProfile(const std::string & profile) override
        {
            UNREFERENCED_PARAMETER(profile);
            return STATUS_ENOSYS;
        }

        virtual status_t LoadTransmitProfiles(const std::string & profiles_json) override
        {
            UNREFERENCED_PARAMETER(profiles_json);
            return STATUS_ENOSYS;
        }

        virtual status_t LoadTransmitProfiles(const std::vector<TransmitProfileRules>& profiles) noexcept override
        {
            UNREFERENCED_PARAMETER(profiles);
            return STATUS_ENOSYS;
        }

        virtual status_t ResetTransmitProfiles() override
        {
            return STATUS_ENOSYS;
        }

        virtual ILogConfiguration& GetLogConfiguration() override
        {
            static ILogConfiguration nullConfig;
            return nullConfig;
        }

        virtual const std::string & GetTransmitProfileName() override
        {
            static std::string nothing;
            return nothing;
        }

        virtual ISemanticContext & GetSemanticContext() override
        {
            static NullContext nullContext;
            return nullContext;
        }

        virtual status_t SetContext(std::string const & /*name*/, std::string const & /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, const char * /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, double /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, int64_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, int8_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, int16_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, int32_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, uint8_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }
        virtual status_t SetContext(const std::string & /*name*/, uint16_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, uint32_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, uint64_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, bool /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, time_ticks_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual status_t SetContext(const std::string & /*name*/, GUID_t /*value*/, PiiKind /*piiKind*/ = PiiKind_None) override
        {
            return STATUS_ENOSYS;
        }

        virtual ILogger * GetLogger(std::string const & /*tenantToken*/, std::string const & /*source*/ = std::string(), std::string const & /*experimentationProject*/ = std::string()) override
        {
            static NullLogger nullLogger;
            return &nullLogger;
        }

        virtual void AddEventListener(DebugEventType /*type*/, DebugEventListener & /*listener*/) override {};

        virtual void RemoveEventListener(DebugEventType /*type*/, DebugEventListener & /*listener*/) override {};

        virtual bool AttachEventSource(DebugEventSource & /*other*/) override
        {
            return false;
        }

        ///
        virtual bool DetachEventSource(DebugEventSource & /*other*/) override
        {
            return false;
        }

        virtual LogSessionData * GetLogSessionData() override
        {
            return nullptr;
        }

        virtual void ResetLogSessionData() override {};

        virtual ILogController* GetLogController() override
        {
            return this;
        }

        virtual IAuthTokensController * GetAuthTokensController() override
        {
            return nullptr;
        }

        virtual IEventFilterCollection& GetEventFilters() noexcept override
        {
            return m_filters;
        }

        virtual const IEventFilterCollection& GetEventFilters() const noexcept override
        {
            return m_filters;
        }

        virtual void SetLevelFilter(uint8_t /*defaultLevel*/, uint8_t /*levelMin*/, uint8_t /*levelMax*/) override {};

        virtual void SetLevelFilter(uint8_t /*defaultLevel*/, const std::set<uint8_t>& /*allowedLevels*/) override {};

        virtual const IDataViewerCollection& GetDataViewerCollection() const noexcept override
        {
            return nullDataViewerCollection;
        }

        virtual IDataViewerCollection& GetDataViewerCollection() noexcept override
        {
            return nullDataViewerCollection;
        }


        void SetDataInspector(const std::shared_ptr<IDataInspector>& /*dataInspector*/) override {}

        void RemoveDataInspector(const std::string& /*name*/) override {}

        void ClearDataInspectors() override {}

        std::shared_ptr<IDataInspector> GetDataInspector(const std::string& /*name*/) noexcept override
        {
            return nullptr;
        }

        virtual status_t DeleteData() noexcept override
        {
            return STATUS_ENOSYS;
        }

        virtual void PauseActivity() override {}
        virtual void ResumeActivity() override {}
        virtual void WaitPause() override {}
        virtual bool StartActivity() override
        {
            return true;
        }
        virtual void EndActivity() override {}

        private:
            NullDataViewerCollection nullDataViewerCollection;
            NullEventFilterCollection m_filters;
    };

} MAT_NS_END

#endif

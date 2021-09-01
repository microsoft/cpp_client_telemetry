//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"
#include "ILogger.hpp"

namespace MATW_NS_BEGIN {

    public interface class ILogger
    {
#ifdef WIN10_CS
        [::Windows::Foundation::Metadata::DefaultOverloadAttribute]
#endif
        virtual void LogEvent(String^ eventName) = 0;
        virtual void LogEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements) = 0;
        virtual void LogEvent(EventProperties^ properties) = 0;
        virtual TimedEvent^ StartTimedEvent(String^ eventName) = 0;
        virtual TimedEvent^ StartTimedEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements) = 0;

        virtual void LogPageView(String^ id, String^ pageName, String^ category, String^ uri, String^ referrerUri, EventProperties^ properties) = 0;
        virtual void LogPageAction(PageActionData^ pageActionData, EventProperties^ properties) = 0;
        virtual void LogAppLifecycle(AppLifeCycleState state, EventProperties^ properties) = 0;
        void virtual LogFailure(String^ signature, String^ detail, String^ category, String^ id, EventProperties^ properties) = 0;
        virtual void LogSampledMetric(String^ name, double value, String^ units, String^ instanceName, String^ objectClass, String^ objectId, EventProperties^ properties) = 0;
        virtual void LogAggregatedMetric(AggregatedMetricData^ metricData, EventProperties^ properties) = 0;
        virtual void LogTrace(TraceLevel level, String^ message, EventProperties^ properties) = 0;
        virtual void LogUserState(UserState state, int64_t timeToLiveInMillis, EventProperties^ properties) = 0;
        virtual void LogSession(SessionState state, EventProperties^ properties) = 0;

        property ISemanticContext^ SemanticContext
        {
            virtual ISemanticContext^ get() = 0;
        }

        virtual void SetContext(String^ name, String^ value) = 0;
        virtual void SetContext(String^ name, String^ value, PiiKind piiKind) = 0;
    };

    /// @cond INTERNAL_DOCS
    /// Excluded from public docs
    public ref class Logger sealed : ILogger
    {
    public:
        virtual void LogEvent(String^ eventName);
        virtual void LogEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);
        virtual void LogEvent(EventProperties^ properties);
        virtual TimedEvent^ StartTimedEvent(String^ eventName);
        virtual TimedEvent^ StartTimedEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);

        virtual void LogPageView(String^ id, String^ pageName, String^ category, String^ uri, String^ referrerUri, EventProperties^ properties);
        virtual void LogPageAction(PageActionData^ pageActionData, EventProperties^ properties);
        virtual void LogAppLifecycle(AppLifeCycleState state, EventProperties^ properties);
        void virtual LogFailure(String^ signature, String^ detail, String^ category, String^ id, EventProperties^ properties);

        virtual void LogSampledMetric(String^ name, double value, String^ units, String^ instanceName, String^ objectClass, String^ objectId, EventProperties^ properties);
        virtual void LogAggregatedMetric(AggregatedMetricData^ metricData, EventProperties^ properties);
        virtual void LogTrace(TraceLevel level, String^ message, EventProperties^ properties);
        virtual void LogUserState(UserState state, int64_t timeToLiveInMillis, EventProperties^ properties);
        virtual void LogSession(SessionState state, EventProperties^ properties);

        property ISemanticContext^ SemanticContext
        {
            virtual ISemanticContext^ get();
        }

        virtual void SetContext(String^ name, String^ value);
        virtual void SetContext(String^ name, String^ value, PiiKind piiKind);

    internal:
        Logger(MAT::ILogger* loggerCore);
        MAT::ILogger* m_loggerCore;

    };
    /// @endcond

} MATW_NS_END


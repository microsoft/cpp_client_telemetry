//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pch.h"

#include "EventPropertiesCX.hpp"
#include "PageActionData.h"
#include "TimedEvent.hpp"
#include "SemanticContextCX.hpp"
#include "AggregatedMetricData.hpp"

#include "LogManagerCX.hpp"
#include "PlatformHelpers.h"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                void CheckRequiredParam(String^ value, String^ parameterName)
                {
                    if (value == nullptr || IsPlatformStringEmpty(value))
                    {
                        ThrowPlatformInvalidArgumentException(parameterName + L" is required.");
                    }
                }

                void CheckRequiredParam(Object ^value, String^ parameterName)
                {
                    if (value == nullptr)
                    {
                        ThrowPlatformInvalidArgumentException(parameterName + L" is required.");
                    }
                }

                void CheckRequiredParam(void* value, String^ parameterName)
                {
                    if (value == nullptr)
                    {
                        ThrowPlatformInvalidArgumentException(parameterName + L" is required.");
                    }
                }

                void Logger::SetContext(String^ name, String ^ value)
                {
                    m_loggerCore->SetContext(FromPlatformString(name), FromPlatformString(value));
                }

                void Logger::SetContext(String ^ name, String ^ value, PiiKind piiKind)
                {
                    m_loggerCore->SetContext(FromPlatformString(name), FromPlatformString(value), (Events::PiiKind)piiKind);
                }

                Logger::Logger(MAT::ILogger* loggerCore) : m_loggerCore(loggerCore)
                {
                    CheckRequiredParam(loggerCore, L"loggerCore");
                }

                void Logger::LogEvent(String^ eventName)
                {
                    LogEvent(eventName, nullptr, nullptr);
                }

                void Logger::LogEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements)
                {
                    CheckRequiredParam(eventName, "eventName");

                    MAT::EventProperties propertiesCore(FromPlatformString(eventName));

                    if (properties != nullptr)
                    {
                        map<string, string> _properties;
                        FromPlatformMap(properties, _properties);

                        for (auto it = _properties.begin(); it != _properties.end(); ++it)
                        {
                            propertiesCore.SetProperty(it->first, it->second);
                        }
                    }

                    if (measurements != nullptr)
                    {
                        map<string, double> _measurements;
                        FromPlatformMap(measurements, _measurements);

                        for (auto it = _measurements.begin(); it != _measurements.end(); ++it)
                        {
                            propertiesCore.SetProperty(it->first, it->second);
                        }
                    }

                    m_loggerCore->LogEvent(propertiesCore);
                }

                void Logger::LogEvent(EventProperties^ eventProperties)
                {
                    CheckRequiredParam(eventProperties, "eventProperties");

                    MAT::EventProperties propertiesCore("");
                    eventProperties->PopulateEventProperties(propertiesCore);

                    m_loggerCore->LogEvent(propertiesCore);
                }

                void Logger::LogPageView(String^ id, String^ pageName, String^ category,
                    String^ uri, String^ referrerUri, EventProperties^ eventProperties)
                {
                    CheckRequiredParam(id, "id");

                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogPageView(FromPlatformString(id), FromPlatformString(pageName),
                        FromPlatformString(category), FromPlatformString(uri), FromPlatformString(referrerUri), propertiesCore);
                }

                void Logger::LogPageAction(PageActionData^ pageActionData, EventProperties^ eventProperties)
                {
                    CheckRequiredParam(pageActionData, "pageActionData");
                    CheckRequiredParam(pageActionData->PageViewId, "PageViewId");

                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    MAT::PageActionData pageActionDataCore(FromPlatformString(pageActionData->PageViewId),
                        (MAT::ActionType)pageActionData->ActionType);

                    pageActionDataCore.rawActionType = (MAT::RawActionType)pageActionData->RawActionType;
                    pageActionDataCore.inputDeviceType = (MAT::InputDeviceType)pageActionData->InputDeviceType;
                    pageActionDataCore.targetItemId = FromPlatformString(pageActionData->TargetItemId);
                    pageActionDataCore.targetItemDataSourceName = FromPlatformString(pageActionData->TargetItemDataSourceName);
                    pageActionDataCore.targetItemDataSourceCategory = FromPlatformString(pageActionData->TargetItemDataSourceCategory);
                    pageActionDataCore.targetItemDataSourceCollection = FromPlatformString(pageActionData->TargetItemDataSourceCollection);
                    pageActionDataCore.targetItemLayoutContainer = FromPlatformString(pageActionData->TargetItemLayoutContainer);
                    pageActionDataCore.targetItemLayoutRank = pageActionData->TargetItemLayoutRank;
                    pageActionDataCore.destinationUri = FromPlatformString(pageActionData->DestinationUri);

                    m_loggerCore->LogPageAction(pageActionDataCore, propertiesCore);
                }

                void Logger::LogAppLifecycle(AppLifeCycleState state, EventProperties^ eventProperties)
                {
                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogAppLifecycle((MAT::AppLifecycleState)state, propertiesCore);
                }

                void Logger::LogFailure(String^ signature, String^ detail, String^ category, String^ id, EventProperties^ eventProperties)
                {
                    CheckRequiredParam(signature, "Signature");
                    CheckRequiredParam(detail, "Detail");

                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogFailure(FromPlatformString(signature), FromPlatformString(detail),
                        FromPlatformString(category), FromPlatformString(id), propertiesCore);
                }

                void Logger::LogSampledMetric(String^ name, double value, String^ units, String^ instanceName,
                    String^ objectClass, String^ objectId, EventProperties^ eventProperties)
                {
                    CheckRequiredParam(name, "name");
                    CheckRequiredParam(units, "units");

                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogSampledMetric(FromPlatformString(name), value, FromPlatformString(units),
                        FromPlatformString(instanceName), FromPlatformString(objectClass), FromPlatformString(objectId), propertiesCore);
                }

                void Logger::LogAggregatedMetric(AggregatedMetricData^ metricData, EventProperties^ eventProperties)
                {
                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    MAT::AggregatedMetricData metricDataCore(FromPlatformString(metricData->Name),
                        (long)metricData->Duration, (long)metricData->Count);

                    metricDataCore.units = FromPlatformString(metricData->Units);
                    metricDataCore.instanceName = FromPlatformString(metricData->InstanceName);
                    metricDataCore.objectClass = FromPlatformString(metricData->ObjectClass);
                    metricDataCore.objectId = FromPlatformString(metricData->ObjectId);
                    FromPlatformMap(metricData->Aggregates, metricDataCore.aggregates);
                    FromPlatformMap(metricData->Buckets, metricDataCore.buckets);

                    m_loggerCore->LogAggregatedMetric(metricDataCore, propertiesCore);
                }

                void Logger::LogTrace(TraceLevel level, String^ message, EventProperties^ eventProperties)
                {
                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogTrace((MAT::TraceLevel)level, FromPlatformString(message), propertiesCore);
                }

                void Logger::LogUserState(UserState state, int64_t timeToLiveInMillis, EventProperties^ eventProperties)
                {
                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogUserState((MAT::UserState)state, (long)timeToLiveInMillis, propertiesCore);
                }

                void Logger::LogSession(SessionState state, EventProperties^ eventProperties)
                {
                    MAT::EventProperties propertiesCore("");
                    if (eventProperties != nullptr)
                    {
                        eventProperties->PopulateEventProperties(propertiesCore);
                    }

                    m_loggerCore->LogSession((MAT::SessionState)state, propertiesCore);
                }
                TimedEvent^ Logger::StartTimedEvent(String^ eventName)
                {
                    return platform_new TimedEvent(this, eventName, nullptr, nullptr);
                }

                TimedEvent^ Logger::StartTimedEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements)
                {
                    return platform_new TimedEvent(this, eventName, properties, measurements);
                }

                ISemanticContext^ Logger::SemanticContext::get()
                {
                    return platform_new SemanticContextImpl(m_loggerCore->GetSemanticContext());
                }
            }
        }
    }
}

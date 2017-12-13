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
                    m_loggerCore->SetContext(FromPlatformString(name), FromPlatformString(value), (Telemetry::PiiKind)piiKind);
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
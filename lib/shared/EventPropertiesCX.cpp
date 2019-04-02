#include "pch.h"
#include "EventPropertiesCX.hpp"
#include "LogManager.hpp"

#define WINDOWS_TICK_MILLISEC 10000LL
#define MILLISEC_TO_UNIVERSAL_EPOCH 11644473600000LL

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                void EventProperties::PopulateEventProperties(MAT::EventProperties& propertiesCore)
                {
                    if (!IsPlatformStringEmpty(this->Name))
                    {
                        propertiesCore.SetName(FromPlatformString(this->Name));
                    }

                    if (!IsPlatformStringEmpty(this->Type))
                    {
                        propertiesCore.SetType(FromPlatformString(this->Type));
                    }

                    if (GetPlatformDateTime(this->Timestamp) != 0)
                    {
                        propertiesCore.SetTimestamp(GetPlatformDateTime(this->Timestamp) / WINDOWS_TICK_MILLISEC - MILLISEC_TO_UNIVERSAL_EPOCH);
                    }

                    propertiesCore.SetPriority((MAT::EventPriority)this->Priority);
                    propertiesCore.SetPolicyBitFlags(this->PolicyBitFlags);

                    map<string, double> measurements;
                    map<string, string> properties;
                    map<string, MAT::PiiKind> piiTags;

                    FromPlatformMap(this->Properties, properties);
                    FromPlatformMap(this->Measurements, measurements);
                    FromPlatformMap(this->PIITags, piiTags);

                    // Not supported in VS2010: for (auto tag : piiTags) 
                    // Not supported in C++/CLI: std::for_each(properties.begin(), properties.end(), [&](pair<string, string> p)
                    for (auto it = piiTags.begin(); it != piiTags.end(); ++it)
                    {
                        if (properties.find(it->first) == properties.end())
                        {
                            ThrowPlatformInvalidArgumentException(ToPlatformString("PII tag '" + it->first + "' does not match any property."));
                        }
                    }

                    for (auto it = properties.begin(); it != properties.end(); ++it)
                    {
                        auto piiType = MAT::PiiKind_None;
                        auto tag = piiTags.find(it->first);

                        if (tag != piiTags.end())
                        {
                            piiType = tag->second;
                        }

                        propertiesCore.SetProperty(it->first, it->second, piiType);
                    }

                    for (auto it = measurements.begin(); it != measurements.end(); ++it)
                    {
                        propertiesCore.SetProperty(it->first, it->second);
                    }

                    if (propertiesCore.GetProperties().size() > 0 && propertiesCore.GetName().empty())
                    {
                        ThrowPlatformInvalidArgumentException(L"Name is required when the event contains custom properties.");
                    }
                }

                EventProperties::EventProperties(const MAT::EventProperties& propertiesCore)
                {
                    this->Name = ToPlatformString(propertiesCore.GetName());

                    this->Type = ToPlatformString(propertiesCore.GetType());

                    // Universal time: represents a point in time as the number of 100-nanosecond intervals prior to or after midnight on January 1, 1601
                    // Unix time: represents number of seconds that have elapsed since 00:00:00 Coordinated Universal Time (UTC), Thursday, 1 January 1970
                    // 1 millisecond = 1,000,000 nanoseconds. 1 milliseconds = 10,000 in 100-nanosecond intervals (WINDOWS_TICK_MILLISEC)
                    // Event core timestamp: Unix time in milliseconds.
                    this->Timestamp = SetPlatformDateTime((propertiesCore.GetTimestamp() + MILLISEC_TO_UNIVERSAL_EPOCH) * WINDOWS_TICK_MILLISEC);

                    // TODO: GetType is not available yet.
                    // m_eventType = ToPlatformString(evt.GetType());

                    // TODO: Filter the contextual properties.
                    // m_context = ToPlatformPropertyMap(evt.GetProperties());

                    // TODO: Filter the semantic properties.
                    // m_semanticContext = ToPlatformPropertyMap(evt.GetProperties());

                    // TODO: Filter/undecorate custom properties.
                    this->Properties = ToPlatformEditablePropertyMap(propertiesCore.GetProperties());

                    // TODO: Extract PII properties.
                    // m_piiProperties = ToPlatformPropertyMap(evt.GetPiiProperties());

                    // TODO: Filter/undecorate measurements.
                    // m_measurements = ToPlatformEditableMeasurementMap(evt.GetProperties());
                }

                EventProperties::EventProperties(String ^ eventName, PropertyMap ^ properties, MeasurementMap ^ measurements)
                {
                    this->Name = eventName;
                    this->Timestamp = ResetPlatformDateTime();
                    // By default we assume that the priority is normal
                    this->Priority = EventPriority::Normal;

                    this->Properties = CreateEditablePropertyMap(properties);
                    this->Measurements = CreateEditableMeasurementMap(measurements);
                    this->PIITags = platform_new PlatfromMap_Underline<String^, PiiKind>();
                }

                EventProperties::EventProperties()
                {
                    this->Init(ToPlatformString(""));
                }

                EventProperties::EventProperties(String ^ name)
                {
                    this->Init(name);
                }

                void EventProperties::Init(String^ name)
                {
                    this->Name = name;
                    this->Timestamp = ResetPlatformDateTime();
                    // By default we assume that the priority is normal
                    this->Priority = EventPriority::Normal;

                    this->Properties = CreateEditablePropertyMap();
                    this->Measurements = CreateEditableMeasurementMap();
                    this->PIITags = platform_new PlatfromMap_Underline<String^, PiiKind>();
                }

                bool EventProperties::SetProperty(String^ key, String^ value)
                {
                    return SetProperty(key, value, MATW::PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, String^ value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    this->Properties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (Properties->HasKey(key))
                        Properties->Remove(key);
                    Properties->Insert(key, value);

                    if (piiKind != MATW::PiiKind::None)
                    {
                        if (PIITags->HasKey(key))
                            PIITags->Remove(key);
                        PIITags->Insert(key, piiKind);
                    }
#endif
                    return true;
                }

                bool EventProperties::SetType(String^ type) {
                    // TODO: [MG] - add validation / consistency check for string
                    this->Type = type;
                    return true;
                }

                String^ EventProperties::GetEventType() {
                    return this->Type;
                }

            }
        }
    }
}
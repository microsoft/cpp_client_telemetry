//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pch.h"
#include "EventPropertiesCX.hpp"
#include "LogManager.hpp"

#pragma warning( push )
#pragma warning( disable : 4454 )

#define WINDOWS_TICK_MILLISEC 10000LL
#define MILLISEC_TO_UNIVERSAL_EPOCH 11644473600000LL

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                template <typename T>
                void EventProperties::StoreEventProperties(MAT::EventProperties& propertiesCore, PlatfromEditableMap<String^, T>^ propertiesMap)
                {
                    map<string, T> properties;
                    map<string, MAT::PiiKind> piiTags;

                    FromPlatformMap(propertiesMap, properties);
                    FromPlatformMap(this->PIITags, piiTags);

                    for (map<string, T>::iterator it = properties.begin(); it != properties.end(); ++it)
                    {
                        MAT::PiiKind piiType = MAT::PiiKind_None;
                        auto tag = piiTags.find(it->first);

                        if (tag != piiTags.end())
                        {
                            piiType = tag->second;
                        }

                        propertiesCore.SetProperty(it->first, static_cast<T>(it->second), piiType);
                    }
                }

                void EventProperties::StoreGuidProperties(MAT::EventProperties& propertiesCore, map<string, MAT::PiiKind>& piiTags)
                {
                    map<string, string> guidProperties;
                    {
#ifdef _WINRT_DLL
                        auto it = this->GuidProperties->First();
                        while (it->HasCurrent)
                        {
                            guidProperties.insert(std::make_pair(FromPlatformString(it->Current->Key), FromPlatformString(it->Current->Value.ToString())));
                            it->MoveNext();
                        }
#else
                        auto it = this->GuidProperties->GetEnumerator();
                        while (it->MoveNext())
                        {
                            guidProperties.insert(std::make_pair(FromPlatformString(it->Current.Key), FromPlatformString(it->Current.Value.ToString())));
                        }
#endif
                    }

                    for (auto it = guidProperties.begin(); it != guidProperties.end(); ++it)
                    {
                        auto piiType = MAT::PiiKind_None;
                        auto tag = piiTags.find(it->first);

                        if (tag != piiTags.end())
                        {
                            piiType = tag->second;
                        }

                        propertiesCore.SetProperty(it->first, it->second, piiType);
                    }
                }

                void EventProperties::StoreDateTimeProperties(MAT::EventProperties& propertiesCore, map<string, MAT::PiiKind>& piiTags)
                {
                    map<string, MAT::time_ticks_t> timeProperties;
                    {
#ifdef _WINRT_DLL
                        auto it = this->DateTimeProperties->First();
                        while (it->HasCurrent)
                        {
                            int64_t ticks = it->Current->Value.UniversalTime;
                            MAT::time_ticks_t dateTime(ticks);
                            timeProperties.insert(std::make_pair(FromPlatformString(it->Current->Key), dateTime));
                            it->MoveNext();
                        }
#else
                        auto it = this->DateTimeProperties->GetEnumerator();
                        while (it->MoveNext())
                        {
                            int64_t ticks = it->Current.Value.Ticks;
                            MAT::time_ticks_t dateTime(ticks);
                            timeProperties.insert(std::make_pair(FromPlatformString(it->Current.Key), dateTime));
                        }
#endif
                    }

                    for (auto it = timeProperties.begin(); it != timeProperties.end(); ++it)
                    {
                        auto piiType = MAT::PiiKind_None;
                        auto tag = piiTags.find(it->first);

                        if (tag != piiTags.end())
                        {
                            piiType = tag->second;
                        }

                        propertiesCore.SetProperty(it->first, it->second, piiType);
                    }
                }

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

                    StoreEventProperties(propertiesCore, this->IntProperties);
                    StoreEventProperties(propertiesCore, this->DoubleProperties);
                    StoreEventProperties(propertiesCore, this->BoolProperties);

                    StoreGuidProperties(propertiesCore, piiTags);
                    StoreDateTimeProperties(propertiesCore, piiTags);


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
                    this->IntProperties = platform_new PlatfromMap_Underline<String^, int64_t>();
                    this->DoubleProperties = platform_new PlatfromMap_Underline<String^, double>();
                    this->BoolProperties = platform_new PlatfromMap_Underline<String^, bool>();
                    this->GuidProperties = platform_new PlatfromMap_Underline<String^, Guid>();
                    this->DateTimeProperties = platform_new PlatfromMap_Underline<String^, DateTime>();
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
                    this->IntProperties = platform_new PlatfromMap_Underline<String^, int64_t>();
                    this->DoubleProperties = platform_new PlatfromMap_Underline<String^, double>();
                    this->BoolProperties = platform_new PlatfromMap_Underline<String^, bool>();
                    this->GuidProperties = platform_new PlatfromMap_Underline<String^, Guid>();
                    this->DateTimeProperties = platform_new PlatfromMap_Underline<String^, DateTime>();
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

                bool EventProperties::SetProperty(String^ key, int64_t value)
                {
                    return SetProperty(key, value, PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, int64_t value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    IntProperties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (IntProperties->HasKey(key))
                        IntProperties->Remove(key);
                    IntProperties->Insert(key, value);

                    if (piiKind != MATW::PiiKind::None)
                    {
                        if (PIITags->HasKey(key))
                            PIITags->Remove(key);
                        PIITags->Insert(key, piiKind);
                    }
#endif
                    return true;
                }

                bool EventProperties::SetProperty(String^ key, double value)
                {
                    return SetProperty(key, value, PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, double value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    DoubleProperties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (DoubleProperties->HasKey(key))
                        DoubleProperties->Remove(key);
                    DoubleProperties->Insert(key, value);

                    if (piiKind != MATW::PiiKind::None)
                    {
                        if (PIITags->HasKey(key))
                            PIITags->Remove(key);
                        PIITags->Insert(key, piiKind);
                    }
#endif
                    return true;
                }

                bool EventProperties::SetProperty(String^ key, bool value)
                {
                    return SetProperty(key, value, PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, bool value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    BoolProperties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (BoolProperties->HasKey(key))
                        BoolProperties->Remove(key);
                    BoolProperties->Insert(key, value);

                    if (piiKind != MATW::PiiKind::None)
                    {
                        if (PIITags->HasKey(key))
                            PIITags->Remove(key);
                        PIITags->Insert(key, piiKind);
                    }
#endif
                    return true;
                }

                bool EventProperties::SetProperty(String^ key, Guid value)
                {
                    return this->SetProperty(key, value, PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, Guid value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    GuidProperties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (GuidProperties->HasKey(key))
                        GuidProperties->Remove(key);
                    GuidProperties->Insert(key, value);

                    if (piiKind != MATW::PiiKind::None)
                    {
                        if (PIITags->HasKey(key))
                            PIITags->Remove(key);
                        PIITags->Insert(key, piiKind);
                    }
#endif
                    return true;
                }

                bool EventProperties::SetProperty(String^ key, DateTime value)
                {
                    return this->SetProperty(key, value, PiiKind::None);
                }

                bool EventProperties::SetProperty(String^ key, DateTime value, PiiKind piiKind)
                {
#ifdef __cplusplus_cli
                    DateTimeProperties[key] = value;
                    if (piiKind != MATW::PiiKind::None)
                    {
                        this->PIITags[key] = piiKind;
                    }
#else
                    if (DateTimeProperties->HasKey(key))
                        DateTimeProperties->Remove(key);
                    DateTimeProperties->Insert(key, value);

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
#pragma warning( pop )


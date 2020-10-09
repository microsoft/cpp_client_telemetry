//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WIN10_CS
/* This module is compiled only for .NET 4.x managed build */
#include "pch.h"

#include "PlatformHelpers.h"
#include "TimedEvent.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                DateTime SetPlatformDateTime(int64_t universalTime)
                {
                    return DateTime::FromFileTimeUtc(universalTime);
                }

                int64_t GetPlatformDateTime(DateTime dt)
                {
                    return dt.ToFileTimeUtc();
                }

                DateTime ResetPlatformDateTime()
                {
                    return DateTime::FromFileTimeUtc(0);
                }

                EditablePropertyMap^ CreateEditablePropertyMap(PropertyMap^ source)
                {
                    auto map = gcnew Dictionary<String^, String^>();

                    if (source != nullptr)
                    {
                        auto it = source->GetEnumerator();
                        while (it->MoveNext())
                        {
                            map->Add(it->Current.Key, it->Current.Value);
                        }
                    }

                    return map;
                }

                EditableMeasurementMap^ CreateEditableMeasurementMap(MeasurementMap^ source)
                {
                    auto map = gcnew Dictionary<String^, double>();

                    if (source != nullptr)
                    {
                        auto it = source->GetEnumerator();
                        while (it->MoveNext())
                        {
                            map->Add(it->Current.Key, it->Current.Value);
                        }
                    }

                    return map;
                }

                EditablePropertyMap^ ToPlatformEditablePropertyMap(const std::map<std::string, std::string>& map)
                {
                    return ToPlatformPropertyMap(map);
                }

                EditablePropertyMap^ ToPlatformEditablePropertyMap(const std::map<std::string, MAT::EventProperty>& map)
                {
                    std::map<std::string, std::string> m;
                    for (auto &kv : map) {
                        auto k = kv.first;
                        auto v = kv.second;
                        m[k] = v.to_string();
                    }
                    return ToPlatformEditablePropertyMap(m);
                }

                EditableMeasurementMap^ ToPlatformEditableMeasurementMap(const std::map<std::string, double>& map)
                {
                    return ToPlatformMeasurementMap(map);
                }

                void ThrowPlatformInvalidArgumentException(String^ message)
                {
                    UNREFERENCED_PARAMETER(message);
#ifndef NDEBUG
                    throw gcnew System::ArgumentException(message);
#endif
                }

                void ThrowPlatformException(String^ message)
                {
                    UNREFERENCED_PARAMETER(message);
#ifndef NDEBUG
                    throw gcnew Exception(message);
#endif
                }

                bool IsPlatformStringEmpty(String^ platformString)
                {
                    return String::IsNullOrEmpty(platformString);
                }

                MeasurementMap^ ToPlatformMeasurementMap(const std::map<std::string, double>& map)
                {
                    Dictionary<String^, double>^ platformMap = gcnew Dictionary<String^, double>();

                    for (auto iter = map.cbegin(); iter != map.cend(); ++iter)
                    {
                        platformMap->Add(ToPlatformString(iter->first), iter->second);
                    }

                    return platformMap;
                }

                PropertyMap^ ToPlatformPropertyMap(const std::map<std::string, std::string>& map)
                {
                    Dictionary<String^, String^>^ platformMap = gcnew Dictionary<String^, String^>();
                    for (auto iter = map.cbegin(); iter != map.cend(); ++iter)
                    {
                        platformMap->Add(ToPlatformString(iter->first), ToPlatformString(iter->second));
                    }

                    return platformMap;
                }
            }
        }
    }
}
#endif

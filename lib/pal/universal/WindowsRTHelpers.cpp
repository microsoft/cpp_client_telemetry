//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef WIN10_CS
/* Helpers for Win 10 C# Universal API */
#include "pch.h"
#include "LogManager.hpp"
#include "PlatformHelpers.h"
//#include "TimedEvent.hpp"

namespace Microsoft {
    namespace Applications {
        // TODO: [MG] - refactor this to use the macro namespace
        namespace Telemetry {
            namespace Windows
            {
                DateTime SetPlatformDateTime(int64_t universalTime)
                {
                    DateTime dt;
                    dt.UniversalTime = universalTime;
                    return dt;
                }

                int64_t GetPlatformDateTime(DateTime dt)
                {
                    return dt.UniversalTime;
                }

                DateTime ResetPlatformDateTime()
                {
                    DateTime dt;
                    dt.UniversalTime = 0;
                    return dt;
                }

                EditablePropertyMap^ CreateEditablePropertyMap(PropertyMap^ source)
                {
                    auto map = ref new Map<String^, String^>();

                    if (source != nullptr)
                    {
                        auto it = source->First();
                        while (it->HasCurrent)
                        {
                            map->Insert(it->Current->Key, it->Current->Value);
                            it->MoveNext();
                        }
                    }

                    return map;
                }

                EditableMeasurementMap^ CreateEditableMeasurementMap(MeasurementMap^ source)
                {
                    auto map = ref new Map<String^, double>();

                    if (source != nullptr)
                    {
                        auto it = source->First();
                        while (it->HasCurrent)
                        {
                            map->Insert(it->Current->Key, it->Current->Value);
                            it->MoveNext();
                        }
                    }

                    return map;
                }

                EditableMeasurementMap^ ToPlatformEditableMeasurementMap(const std::map<std::string, double>& map)
                {
                    Map<String^, double>^ platformMap = ref new Map<String^, double>();

                    std::for_each(map.begin(), map.end(), [&](std::pair<std::string, double> m)
                    {
                        platformMap->Insert(ToPlatformString(m.first), m.second);
                    });

                    return platformMap;
                }

                MeasurementMap^ ToPlatformMeasurementMap(const std::map<std::string, double>& map)
                {
                    return ToPlatformEditableMeasurementMap(map)->GetView();
                }

                EditablePropertyMap^ ToPlatformEditablePropertyMap(const std::map<std::string, std::string>& map)
                {
                    Map<String^, String^>^ platformMap = ref new Map<String^, String^>();

                    std::for_each(map.begin(), map.end(), [&](std::pair<std::string, std::string> m)
                    {
                        platformMap->Insert(ToPlatformString(m.first), ToPlatformString(m.second));
                    });

                    return platformMap;
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

                PropertyMap^ ToPlatformPropertyMap(const std::map<std::string, std::string>& map)
                {
                    return ToPlatformEditablePropertyMap(map)->GetView();
                }

                void ThrowPlatformInvalidArgumentException(String^ message)
                {
#ifndef NDEBUG 
                    throw ref new InvalidArgumentException(message);
#endif
                }

                void ThrowPlatformException(String^ message)
                {
#ifndef NDEBUG 
                    throw ref new Exception(-1, message);
#endif
                }

                bool IsPlatformStringEmpty(String^ platformString)
                {
                    return platformString != nullptr ? platformString->IsEmpty() : true;
                }

                void FromPlatformMap(PropertyMap^ platformMap, std::map<std::string, std::string>& map)
                {
                    if (platformMap != nullptr)
                    {
                        auto iter = platformMap->First();
                        while (iter->HasCurrent)
                        {
                            map[FromPlatformString(iter->Current->Key)] = FromPlatformString(iter->Current->Value);
                            iter->MoveNext();
                        }
                    }
                }

                void FromPlatformMap(MeasurementMap^ platformMap, std::map<std::string, double>& map)
                {
                    if (platformMap != nullptr)
                    {
                        auto iter = platformMap->First();
                        while (iter->HasCurrent)
                        {
                            map[FromPlatformString(iter->Current->Key)] = iter->Current->Value;
                            iter->MoveNext();
                        }
                    }
                }

                /**
                 * Calculate SHA1 based on token name to shorten the filename
                 */
                using namespace ::Windows::Security::Cryptography;
                using namespace ::Windows::Storage::Streams;
                using namespace ::Windows::Security::Cryptography::Core;
                String^ sha1(String ^input) {
                    IBuffer ^buffer = CryptographicBuffer::ConvertStringToBinary(input, BinaryStringEncoding::Utf8);
                    HashAlgorithmProvider ^hashAlgorithm = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha1);
                    IBuffer ^hashBuffer = hashAlgorithm->HashData(buffer);
                    String ^result = CryptographicBuffer::EncodeToBase64String(hashBuffer);
                    return result;
                }

                /**
                 * Remove illegal characters from filename
                 */
                String^ safeFilename(String ^input) {
                    std::string forbidden = "\\/:?\"<>|=";
                    std::string s = FromPlatformString(input);
                    for (auto it = s.begin(); it < s.end(); ++it) {
                        bool found = forbidden.find(*it) != string::npos;
                        // Replace all forbidden symbols by '-'
                        if (found) {
                            *it = '-';
                        }
                    }
                    return ToPlatformString(s);
                }

            }
        }
    }
}
#endif

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef PLATFORMHELPERS_HPP
#define PLATFORMHELPERS_HPP

#include <ctmacros.hpp>

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#include <codecvt>

#ifdef __cplusplus_winrt
#ifndef _WINRT_DLL
#define _WINRT_DLL
#endif
#endif

#ifdef _WINRT_DLL
#include <collection.h>

using namespace ::Windows::Foundation;
using namespace Platform::Collections;
using namespace ::Windows::Foundation::Collections;
using namespace Platform;

typedef IMapView<String^, String^> PropertyMap;
typedef IMapView<String^, double> MeasurementMap;
typedef IMap<String^, String^> EditablePropertyMap;
typedef IMap<String^, double> EditableMeasurementMap;
#define PlatformVector  Vector<String^>
#define IPlatformVector IVector<String^>

#define platform_new ref new
#define platform_insert Insert
#define PlatfromEditableMap IMap
#define PlatfromMap IMapView
#define PlatfromMap_Underline Map
#define WstringFromPlatformString(x) x->Data()
#else
#include <msclr\all.h>
#include <msclr\marshal.h>

using namespace System;
using namespace msclr::interop;
using namespace System::Collections::Generic;

typedef IDictionary<String^, String^> PropertyMap;
typedef IDictionary<String^, double> MeasurementMap;
typedef IDictionary<String^, String^> EditablePropertyMap;
typedef IDictionary<String^, double> EditableMeasurementMap;
#define PlatformVector  List<String^>
#define IPlatformVector IList<String^>
#define IVector IList

#define platform_new gcnew
#define platform_insert Add
#define PlatfromEditableMap IDictionary
#define PlatfromMap IDictionary
#define PlatfromMap_Underline Dictionary
#define WstringFromPlatformString(x) msclr::interop::marshal_as<std::wstring>(x)
#endif

#include "utils/Utils.hpp"

using namespace std;

namespace MAT_NS_BEGIN {
    class EventProperties;
    class ILogger;
    class LogManager;
    class ISemanticContext;
    class LogConfiguration;
    struct EventProperty;
} MAT_NS_END;

namespace Microsoft {
    namespace Applications {

        class EventCore;
        class ITestCallback;

        namespace Telemetry
        {
            namespace Windows
            {
                ref class TimedEvent;
                ref class EventProperties;
                ref struct LogConfiguration;
                ref struct PageActionData;
                ref struct AggregatedMetricData;

                interface class ILogger;
                interface class ISemanticContext;

                DateTime ResetPlatformDateTime();
                DateTime SetPlatformDateTime(int64_t universalTime);
                int64_t GetPlatformDateTime(DateTime dt);

                bool IsPlatformStringEmpty(String^ platformString);
                void ThrowPlatformInvalidArgumentException(String^ message);
                void ThrowPlatformException(String^ message);

                // Defining the template function in the header file eliminates the need in additional linker definitions.
                // platformmaptype can be read-only or editable platform map.
                template <typename K1, typename V1, typename K2, typename V2, template<typename, typename> class platformmaptype>
                inline void FromPlatformMap(platformmaptype<K1, V1>^ platformMap, std::map<K2, V2>& map)
                {
                    if (platformMap != nullptr)
                    {
#ifdef _WINRT_DLL
                        auto it = platformMap->First();
                        while (it->HasCurrent)
                        {
                            map.insert(std::make_pair((K2)it->Current->Key, (V2)it->Current->Value));
                            it->MoveNext();
                        }
#else
                        auto it = platformMap->GetEnumerator();
                        while (it->MoveNext())
                        {
                            map.insert(std::make_pair((K2)it->Current.Key, (V2)it->Current.Value));
                        }
#endif
                    }
                }

                template <typename V1, typename V2, template<typename, typename> class platformmaptype>
                inline void FromPlatformMap(platformmaptype<String^, V1>^ platformMap, map<string, V2>& map)
                {
                    if (platformMap != nullptr)
                    {
#ifdef _WINRT_DLL
                        auto it = platformMap->First();
                        while (it->HasCurrent)
                        {
                            map.insert(std::make_pair(FromPlatformString(it->Current->Key), (V2)it->Current->Value));
                            it->MoveNext();
                        }
#else
                        auto it = platformMap->GetEnumerator();
                        while (it->MoveNext())
                        {
                            map.insert(std::make_pair(FromPlatformString(it->Current.Key), (V2)it->Current.Value));
                        }
#endif
                    }
                }

                template <template<typename, typename> class platformmaptype>
                inline void FromPlatformMap(platformmaptype<String^, String^>^ platformMap, std::map<string, string>& map)
                {
                    if (platformMap != nullptr)
                    {
#ifdef _WINRT_DLL
                        auto it = platformMap->First();
                        while (it->HasCurrent)
                        {
                            map.insert(std::make_pair(FromPlatformString(it->Current->Key), FromPlatformString(it->Current->Value)));
                            it->MoveNext();
                        }
#else
                        auto it = platformMap->GetEnumerator();
                        while (it->MoveNext())
                        {
                            map.insert(std::make_pair(FromPlatformString(it->Current.Key), FromPlatformString(it->Current.Value)));
                        }
#endif
                    }
                }

                // Convert a wide Unicode string to an UTF8 string
                inline string WstringToUTF8(const wstring& wstr)
                {
                    if (wstr.empty())
                    {
                        return std::string();
                    }
                    using convert_type = std::codecvt_utf8<wchar_t>;
                    std::wstring_convert<convert_type, wchar_t> converter;
                    std::string result = converter.to_bytes(wstr);
                    return result;
                }

                // Convert an UTF8 string to a wide Unicode String
                inline wstring UTF8ToWstring(const string& str)
                {
                    if (str.empty())
                    {
                        return wstring();
                    }

                    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
                    std::wstring wstrTo(size, 0);
                    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size);

                    return wstrTo;
                }

                inline std::string FromPlatformString(String^ platformString)
                {
                    if (platformString != nullptr)
                    {
#ifdef _WINRT_DLL
                        wstring ws = WstringFromPlatformString(platformString);
                        return WstringToUTF8(ws);
#else
                        marshal_context^ context = gcnew marshal_context();
                        String^ message = gcnew String(platformString);
                        const char* result = context->marshal_as<const char*>(message);
                        std::string r(result, strlen(result));
                        delete context;
                        return r;
#endif
                    }
                    return std::string();
                }

                inline String^ ToPlatformString(const std::string& string)
                {
                    return platform_new String(UTF8ToWstring(string).c_str());
                }

                PropertyMap^ ToPlatformPropertyMap(const std::map<std::string, std::string>& map);

                MeasurementMap^ ToPlatformMeasurementMap(const std::map<std::string, double>& map);

                EditablePropertyMap^ ToPlatformEditablePropertyMap(const std::map<std::string, std::string>& map);

                EditablePropertyMap^ ToPlatformEditablePropertyMap(const std::map<std::string, MAT::EventProperty>& map);

                EditableMeasurementMap^ ToPlatformEditableMeasurementMap(const std::map<std::string, double>& map);

                EditablePropertyMap^ CreateEditablePropertyMap(PropertyMap^source = nullptr); // ref new Map<String^, String^>();

                EditableMeasurementMap^ CreateEditableMeasurementMap(MeasurementMap^source = nullptr); // ref new Map<String^, double>();

                inline IPlatformVector^ ToPlatformVector(const std::vector<std::string>& v) {
                    PlatformVector^ result = platform_new PlatformVector();
                    for (const auto& key : v) {
                        String ^k = ToPlatformString(key);
#ifdef _WINRT_DLL
                        result->Append(k);
#else
                        result->Add(k);
#endif
                    }
                    return result;
                }

            }
        }
    }
}

namespace MATW = Microsoft::Applications::Telemetry::Windows;
#define MATW_NS_BEGIN Microsoft { namespace Applications { namespace Telemetry { namespace Windows
#define MATW_NS_END   }}}

#endif

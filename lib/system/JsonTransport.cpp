#if 0
// Copyright (c) Microsoft. All rights reserved.

/**
 * Minimalistic implementation of Aria SDK synchronous HTTP POST in JSON format.
 * This can be used as a standalone implementation, but it does not provide offline
 * storage and batching.
 */

#include "http/HttpClientFactory.hpp"
#include "LogManager.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "json.hpp"
using json = nlohmann::json;

#ifdef _MSC_VER
#pragma warning( disable : 4189 4100)
#endif

namespace PAL_NS_BEGIN {
    std::string getOsBuildLabEx();
    void        getOsVersion(std::string& osMajorVersion, std::string& osFullVersion);
    std::string getAppId();
    std::string getAppVersion();
    const char* getDeviceId();
} PAL_NS_END

namespace ARIASDK_NS_BEGIN {

    /**
     * Get current time in millis
     */
    __inline unsigned long long gettimems()
    {
        unsigned long long temp = 0;
#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL
        SYSTEMTIME st;
        GetSystemTime(&st);
        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);
        temp = *((unsigned long long *)(&ft));
        temp = temp / TICKS_PER_SECOND;
        temp = temp - EPOCH_DIFFERENCE;
        temp = temp * 1000 + st.wMilliseconds;
        return temp;
    };

    static IHttpClient* httpClient = HttpClientFactory::Create();

    class ResponseCallback2 : public IHttpResponseCallback {
    public:
        ResponseCallback2() {};
        virtual void OnHttpResponse(IHttpResponse* response) {
            printf("Response: %u\n", response->GetStatusCode());
        };
    };

    ResponseCallback2 jsonResponseCallback;

    static std::string devId;
    static std::string osVer;
    static std::string appId;
    static std::string appVer;

    /**
     * Populate minimalistic set of common fields
     */
    void populateCommonFields()
    {
        static bool populated = false;
        if (!populated)
        {
            devId = "s:";
            devId += PAL::getDeviceId();

            std::string osVerMajor;
            std::string osVerFull;
            PAL::getOsVersion(osVerMajor, osVerFull);
            osVer = osVerFull;

            appId = PAL::getAppId();
            appVer = PAL::getAppVersion();
        }
    }

    /**
     * Decorate EventProperties for given token and send as JSON
     */
    void SendAsJSON(const EventProperties& props, const std::string& token)
    {
        populateCommonFields();

        static int64_t seq = 0;
        seq++;

        std::string O_key = "o:";
        O_key += tenantTokenToId(token);

        int64_t t = PAL::getUtcSystemTimeMs();
        std::string ts = PAL::formatUtcTimestampMsAsISO8601(t);

        std::string eventName = props.GetName();
        std::replace(eventName.begin(), eventName.end(), '_', '.');

        json j1 =
        {
            { "name", eventName },
            { "time", ts },
            { "iKey", O_key },
            { "ver" , "3.0" },
        };

        json j;

        j["ext"] = json({});

        j["ext"]["os"] =
        {
            { "name", "Windows" },
            { "ver", osVer }
        };

        j["ext"]["sdk"] =
        {
            { "ver", "ACT-C++-OneSDK-2.0.0" },
            { "epoch", "2209784" },
            { "seqNum" , seq }
        };

        j["ext"]["device"] =
        {
            { "localId", "s:F11CFE51-7009-475E-B307-F5086ADBE4B1" },
            { "deviceClass", "Windows.Desktop" },
            // FIXME: these two fields below are not available to Aria SDK
            { "id", "g:6896134429636487" } ,
            { "authId" , "g:6896134429636487" }
        };

        j["ext"]["user"] =
        {
            // FIXME: these two fields below are not available to Aria SDK
            { "localId" , "w:355B7300-07A5-4FE3-5C24-5619C241F373" },
            { "id" , "w:355B7300-07A5-4FE3-5C24-5619C241F373" },
        };

        j["ext"]["app"] =
        {
            { "ver", appVer },
            { "id",  appId },
            { "locale", "en-US" },
            { "user",
                {
                    { "locale", "en-US" },
                    { "loc",
                        {
                            { "timeZone", "-08:00" }
                        }
                    }
                }
            }
        };

        j["data"] = json({});
        j["data"]["baseType"] = "";
        j["data"]["baseData"] = "";
        j["data"]["baseDataType"] = "custom";
        j["data"]["eventName"] = eventName;

        for (auto &kv : props.GetProperties())
        {
            const std::string &key = kv.first;
            const EventProperty &val = kv.second;
            switch (val.type)
            {
            case EventProperty::TYPE_STRING:
                j["data"][key] = val.as_string;
                break;
            case EventProperty::TYPE_INT64:
                j["data"][key] = val.as_int64;
                break;
            case EventProperty::TYPE_DOUBLE:
                j["data"][key] = val.as_double;
                break;
            case EventProperty::TYPE_TIME:
                j["data"][key] = val.as_time_ticks.ticks;
                break;
            case EventProperty::TYPE_BOOLEAN:
                j["data"][key] = val.to_string().c_str();
                break;
            case EventProperty::TYPE_GUID:
                j["data"][key] = val.to_string();
                break;
            }
        };

        j["data"]["DeviceInfo"] = {
            // FIXME: read this from SDK user analytics storage
            { "SDKUid", "2e570610-f98c-4d0a-ac5d-ea23f724da55" },
            { "Id", devId }
        };

        j["data"]["EventInfo"] = {
            // FIXME: change this to random GUID
            { "InitId", "fb441644-7de0-4f1b-b84c-6cfb7e6a09cf" },
            { "Sequence", seq }
        };

        std::string s1 = j1.dump();
        std::string s2 = j.dump();
        std::string s = "{";
        s += s1.substr(1, s1.length() - 2);
        s += ",";
        s += s2.substr(1, s2.length() - 2);
        s += "}";

        printf("%s\n", s.c_str());

        std::vector<uint8_t> body;
        body.insert(body.end(), s.c_str(), s.c_str() + s.length());

        IHttpRequest* request = httpClient->CreateRequest();
        request->SetMethod("POST");

        HttpHeaders& headers = request->GetHeaders();
        headers.add("Accept", "application/json, text/javascript, */*; q=0.01");
        headers.add("Keep-Alive", "true");
        headers.add("Content-Type", "application/x-json-stream");
        headers.add("Content-Length", std::to_string(body.size()));
        headers.add("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36");

        std::string url;
        url = "https://v10.events.data.microsoft.com/OneCollector/1.0?cors=true&client-id=NO_AUTH&client-version=ACT-ONE-SDK-1&";
        // FOR INT:
        // "https://pipe.int.trafficmanager.net/OneCollector/1.0?cors=true&client-id=NO_AUTH&client-version=ACT-ONE-SDK-1&";
        url += "x-apikey=";
        url += token;
        url += "&upload-time=";
        url += std::to_string(t / 1000L);
        url += "&time-delta-to-apply-millis=use-collector-delta";
        request->SetUrl(url);
        request->SetBody(body);
        httpClient->SendRequestAsync(request, &jsonResponseCallback);
    }

} ARIASDK_NS_END
#endif

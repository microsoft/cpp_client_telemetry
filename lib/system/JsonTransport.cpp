// Copyright (c) Microsoft. All rights reserved.

#include "LogManager.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

#include "http/HttpClient_WinInet.hpp"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "json.hpp"
using json = nlohmann::json;

#pragma warning( disable : 4189 4100)

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

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

            static HttpClient_WinInet* httpClient = new HttpClient_WinInet();

            class ResponseCallback2 : public IHttpResponseCallback {
            public:
                ResponseCallback2() {};
                virtual void OnHttpResponse(IHttpResponse const* response) {
                    printf("Response: %u\n", response->GetStatusCode());
                };
            };

            ResponseCallback2 jsonResponseCallback;

            void SendAsJSON(const EventProperties& props, const std::string& token)
            {
                std::string O_key = "O:";
                O_key += tenantTokenToId(token);

                int64_t t = PAL::getUtcSystemTimeMs();
                std::string ts = PAL::formatUtcTimestampMsAsISO8601(t);

                json j =
                {
                    { "name", props.GetName().c_str() },
                    { "time", ts /* "2017-11-10T23:50:03.757Z" */ },
                    { "iKey", O_key },
                    { "ver" , "3.0" }
                };

                j["ext"] = json({});
                j["ext"]["os"] =
                {
                    { "name", "Windows" },
                    { "ver", "10" }
                };

                j["data"] = json({});
                j["data"]["baseData"] = json({});
                j["data"]["baseDataType"] = "custom";

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

                std::string s = j.dump();
                printf("%s\n", s.c_str());

                std::vector<uint8_t> body;
                body.insert(body.end(), s.c_str(), s.c_str() + s.length() );

                IHttpRequest* request = httpClient->CreateRequest();
                request->SetMethod("POST");

                HttpHeaders& headers = request->GetHeaders();
                headers.add("Accept", "application/json, text/javascript, */*; q=0.01");
                headers.add("Keep-Alive", "true");
                headers.add("Content-Type", "application/x-json-stream");
                headers.add("Content-Length", std::to_string(body.size()));
                headers.add("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36");

                std::string url;
                url = "https://pipe.int.trafficmanager.net/OneCollector/1.0?cors=true&client-id=NO_AUTH&client-version=ACT-ONE-SDK-1&";
                url += "x-apikey=";
                url += token;
                url += "&upload-time=";
                url += std::to_string(t / 1000L);
                url += "&time-delta-to-apply-millis=use-collector-delta";
                request->SetUrl(url);
                request->SetBody(body);
                httpClient->SendRequestAsync(request, &jsonResponseCallback);

            }
        }
    }
}
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <fstream>

#include "LogManager.hpp"

using namespace MAT;

// Define it once per .exe or .dll in any compilation module
LOGMANAGER_INSTANCE

// Read configuration and event file contents
std::string readall(const std::string& path)
{
    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return content;
}

#define JSON_CONFIG(...)    ( #__VA_ARGS__ )

// Default configuration in JSON format. Telemetry by default is sent to "1DSCppSdkTest" tenant.
// Please change "primaryToken" parameter below to send telemetry to your subscription.
const char* defaultConfig = static_cast<const char *> JSON_CONFIG
(
    {
        "cacheFileFullNotificationPercentage": 75,
        "cacheFilePath" : "storage.db",
        "cacheFileSizeLimitInBytes" : 3145728,
        "cacheMemoryFullNotificationPercentage" : 75,
        "cacheMemorySizeLimitInBytes" : 524288,
        "compat" : {
            "dotType": true
        },
        "enableLifecycleSession" : false,
        "eventCollectorUri" : "https://self.events.data.microsoft.com/OneCollector/1.0/",
        "forcedTenantToken" : null,
        "hostMode" : true,
        "http" : {
            "compress": true
        },
        "maxDBFlushQueues" : 3,
        "maxPendingHTTPRequests" : 4,
        "maxTeardownUploadTimeInSec" : 1,
        "minimumTraceLevel" : 4,
        "multiTenantEnabled" : true,
        "primaryToken" : "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991",
        "sample" : {
            "rate": 0
        },
        "sdkmode" : 0,
        "skipSqliteInitAndShutdown" : null,
        "stats" : {
            "interval": 1800,
            "split" : false,
            "tokenInt" : "8130ef8ff472405d89d6f420038927ea-0c0d561e-cca5-4c81-90ed-0aa9ad786a03-7166",
            "tokenProd" : "4bb4d6f7cafc4e9292f972dca2dcde42-bd019ee8-e59c-4b0f-a02c-84e72157a3ef-7485"
        },
        "tpm": {
            "backoffConfig": "E,3000,300000,2,1",
            "clockSkewEnabled" : true,
            "maxBlobSize" : 2097152,
            "maxRetryCount" : 5
        },
        "traceLevelMask": 0,
        "utc" : {
            "providerGroupId" : "780dddc8-18a1-5781-895a-a690464fa89c"
        }
    }
);

int main(int argc, char *argv[])
{
    // 2nd (optional) parameter - path to custom SDK configuration
    std::string customConfig;
    if (argc == 2)
    {
        std::string filename = argv[1];
        printf("Reading configuration from %s\n", filename.c_str());
        customConfig = readall(filename);
    }

    // Use custom configuration if available
    const char *jsonConfig = (customConfig.empty()) ? defaultConfig : customConfig.c_str();

    // LogManager configuration
    auto& config = LogManager::GetLogConfiguration();
    config = MAT::FromJSON(jsonConfig);

    // LogManager initialization
    ILogger *logger = LogManager::Initialize();
    bool utcActive = (bool)(config[CFG_STR_UTC][CFG_BOOL_UTC_ACTIVE]);

    printf("Running in %s mode...\n", (utcActive) ? "UTC" : "direct upload");
    if (utcActive)
    {
        printf("UTC provider group Id: %s\n", (const char *)(config[CFG_STR_UTC][CFG_STR_PROVIDER_GROUP_ID]));
        printf("UTC large payloads:    %s\n", ((bool)(config[CFG_STR_UTC][CFG_BOOL_UTC_LARGE_PAYLOADS])) ? "supported" : "not supported");
    }
    else
    {
        printf("Collector URL:         %s\n", (const char *)(config[CFG_STR_COLLECTOR_URL]));
    }

    printf("Token (iKey):          %s\n", (const char *)(config[CFG_STR_PRIMARY_TOKEN]));

#if 0
    // Code example that shows how to convert ILogConfiguration to JSON
    std::string json;
    Variant::serialize(config, json);
    printf("%s\n", json.c_str());
#endif

    // Log simple event without any properties.
    //
    // Note that this event will be blocked in UTC mode on most recent versions of
    // Windows as it has no privacy tag.
    printf("Sending My.Simple.Event\n");
    logger->LogEvent("My.Simple.Event");

    // use current time as seed for random generator
    std::srand((unsigned int)(std::time(nullptr)));

    // try to ingest some 128K+ events
    size_t MAX_EVENT_SIZE = 131072;

    // Log detailed event with some properties
    for (auto eventName : { "My.Detailed.Event", "My.Detailed.Event.PiiMark", "My.Detailed.Event.PiiDrop", "My.Detailed.Event.RPC" })
    {
        EventProperties evt(eventName,
            {
                // Pii-typed fields
                { "piiKind.None",               EventProperty("field_value",  PiiKind_None) },
                { "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
                { "piiKind.GenericData",        EventProperty("generic_data",  PiiKind_GenericData) },
                { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
                { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
                { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
                { "piiKind.PhoneNumber",        EventProperty("+1-425-829-5875", PiiKind_PhoneNumber) },
                { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
                { "piiKind.SipAddress",         EventProperty("sip:info@microsoft.com", PiiKind_SipAddress) },
                { "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@fabrikam.com>", PiiKind_SmtpAddress) },
                { "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
                { "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
                { "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },
                // Various typed key-values
                { "strKey1",  "hello1" },
                { "strKey2",  "hello2" },
                { "int64Key", int64_t(1L) },
                { "dblKey",   3.14 },
                { "boolKey",  false },
                { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
                { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
                { "guidKey2", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
                { "timeKey1", time_ticks_t((uint64_t)0) },     // time in .NET ticks
            });

        if (utcActive)
        {
            // Add privacy tags to avoid the event being dropped at UTC layer
            evt.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_ProductAndServicePerformance);
            evt.SetProperty(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_OPTIONAL);
        }

        if (std::string("My.Detailed.Event.PiiMark") == eventName)
        {
            evt.SetPolicyBitFlags(MICROSOFT_EVENTTAG_MARK_PII);
        }
        else if (std::string("My.Detailed.Event.PiiDrop") == eventName)
        {
            evt.SetPolicyBitFlags(MICROSOFT_EVENTTAG_DROP_PII);
        }
        else if (std::string("My.Detailed.Event.RPC") == eventName)
        {
            // Ingest some UTC 'large' events via RPC channel
            std::string s;
            size_t i = 0;
            while (i++ < MAX_EVENT_SIZE)
            {
                s += char('a' + std::rand() % 26);
            }
            evt.SetProperty("bigString", s);
        }

        printf("Sending %s\n", eventName);
        logger->LogEvent(evt);
    }

    // Shutdown
    printf("FlushAndTeardown...\n");
    LogManager::FlushAndTeardown();
    printf("[ DONE ]\n");

    return 0;
}

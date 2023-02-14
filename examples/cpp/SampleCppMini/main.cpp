#define _CRT_SECURE_NO_WARNINGS

// #define API_KEY   "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999"
#define API_KEY "fba3c287ba474016b77e0ab612175255-8ef3561c-6103-4027-8ca0-2c79dcbd8901-6902"

#include <cstdio>
#include <cstdlib>

#include "LogManager.hpp"

using namespace MAT;

#ifdef _WIN32
#pragma comment(lib, "Ole32.Lib")      /* needed for CoCreateGuid */
#pragma comment(lib, "Advapi32.Lib")   /* needed for RegGetValueA */
#endif

LOGMANAGER_INSTANCE

extern "C" void test_c_api(const char *token);

void test_cpp_api(const char * token, int ticketType, const char *ticket)
{
    printf("Testing C++ API...\t");

    // LogManager configuration
    auto& config = LogManager::GetLogConfiguration();
    // config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;

    // Disable stats event to avoid 403
    config["stats"]["interval"] = 0;
    config["maxTeardownUploadTimeInSec"] = 5;
    config[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
    config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel::ACTTraceLevel_Debug;

    // LogManager initialization
    ILogger *logger = LogManager::Initialize(token);

    // Print configuration
    std::string dumpStr;
    Variant::serialize((VariantMap&)config, dumpStr);
    printf("\nJSON config:\n%s", dumpStr.c_str());

    if (ticket != nullptr)
    {
        const char *ticketNames[8] =
        {
            "TicketType_MSA_Device",
            "TicketType_MSA_User",
            "TicketType_XAuth_Device",
            "TicketType_XAuth_User",
            "TicketType_AAD",
            "TicketType_AAD_User",
            "TicketType_AAD_JWT",
            "TicketType_AAD_Device"
        };
        printf("\nSet ticket %s=%s\n", ticketNames[ticketType], ticket);
        auto tc = LogManager::GetAuthTokensController();
        tc->SetStrictMode(true);
        tc->SetTicketToken((TicketType)ticketType, ticket);
    }

    // Log simple event without any properties
    logger->LogEvent("MyApp.simple_event");

    ISemanticContext *global_ctx = LogManager::GetSemanticContext();
    auto local_ctx = logger->GetSemanticContext();
    logger->SetContext("Local.Context.Variable", "value");

    // Log detailed event with various properties
    EventProperties detailed_event("MyApp.detailed_event",
        {
            // Log compiler version
            { "_MSC_VER", _MSC_VER },
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
            { "int64Key", 1L },
            { "dblKey",   3.14 },
            { "boolKey",  false },
            { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
            { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            { "guidKey2", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            { "timeKey1",  time_ticks_t((uint64_t)0) },     // time in .NET ticks
        });
    logger->LogEvent(detailed_event);

    // Shutdown
    LogManager::UploadNow();
    LogManager::FlushAndTeardown();

    printf("[ DONE ]\n");
}

int main(int argc, const char *argv[])
{
    printf("Microsoft Event Analytics 1DS pipe test tool \n");
    printf("=============================================\n");

    const char *token = (argc > 1) ? argv[1] : API_KEY;
    int  type = (argc > 2) ? atoi(argv[2]) : TicketType::TicketType_MSA_Device;
    const char *ticket = (argc > 3) ? argv[3] : nullptr;

    // Send event using C API
    // test_c_api(token);

    // Send event using C++ API
    test_cpp_api(token, type, ticket);

    return 0;
}

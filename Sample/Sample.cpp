// Sample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
int main()
{
    return 0;
}

*/
//#include <public/ILogManager.hpp>
//#include <aria/DebugEvents.hpp>
#include "public/LogManager.hpp"
#include "public/Enums.hpp"
#include <iostream>

#include <time.h>

#include <thread>
#include <mutex>
#include <vector>
#include <ctime>


// OTEL profile example
const char* transmitProfileDefinitions = R"(
[{
    "name": "Office_Telemetry_OneSecond",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 1 ] },
    { "netCost": "low",        "timers": [ 1, 1, 1 ] },
    { "netCost": "unknown",    "timers": [ 1, 1, 1 ] },
    {                          "timers": [ 1, 1, 1 ] }
    ]
},
{
    "name": "Office_Telemetry_TenSeconds",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 10 ] },
    { "netCost": "low",        "timers": [ 10, 10, 10 ] },
    { "netCost": "unknown",    "timers": [ 10, 10, 10 ] },
    {                          "timers": [ 10, 10, 10 ] }
    ]
},
{
    "name": "Office_Telemetry_OneMinute",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 60 ] },
    { "netCost": "low",        "timers": [ 60, 60, 60 ] },
    { "netCost": "unknown",    "timers": [ 60, 60, 60 ] },
    {                          "timers": [ 60, 60, 60 ] }
    ]
}]
)";

#include <windows.h>  
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR szName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;

#pragma pack(pop)  
void SetThreadName(DWORD dwThreadID, const char* threadName) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)  
}

void SetThreadName(const char* threadName)
{
	SetThreadName(GetCurrentThreadId(), threadName);
}



using namespace Microsoft::Applications::Telemetry;
//using namespace ARIASDK_NS_BEGIN;
using namespace std;

// Specify this API token in the SDK initialization call to send data for this application.
// Please keep this token secure if it is for production services.
// https://aria.microsoft.com/developer/start-now/using-aria/send-events

#define USE_INT
// #define USE_BOGUS_URL

#ifdef USE_INT
// Windows SDK Test - Int: Default Ingestion Token.
#define TOKEN   "0c21c15bdccc48c99678a748488bb87f-cca6848e-b4aa-48a6-b24a-0170caf27523-7582"
//"112b4296adfa44b68570a476ec2b2e1b-b7a01aa4-4f7b-4b2e-ad82-f3f48f15833c-7683"
//"0c21c15bdccc48c99678a748488bb87f-cca6848e-b4aa-48a6-b24a-0170caf27523-7582" // //
// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "462f0c61d59d43d1bf6987688131bd2e-370ca818-e162-499e-a3b8-39e55aad385d-6983"
#else
// Windows SDK Test - Prod: Default Ingestion Token.
#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"
// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "0ae6cd22d8264818933f4857dd3c1472-eea5f30e-e0ed-4ab0-8ed0-4dc0f5e156e0-7385"
#endif

// Windows SDK Test - Prod: Default Ingestion Token.
const string cTenantToken = TOKEN;

std::mutex dbg_callback_mtx;

#ifdef USE_ECG
CounterSink counterSink("127.0.0.1", 8888, CounterSinkProto::UDP);
#endif

const char* networkCostNames[] = {
	"Unknown",
	"Unmetered",
	"Metered",
	"Roaming",
};

std::atomic<unsigned>   eps = 0;
std::atomic<unsigned>   numLogged0 = 0;
std::atomic<unsigned>   numLogged = 0;
std::atomic<unsigned>   numRejected = 0;
std::atomic<unsigned>   numSent = 0;
std::atomic<unsigned>   numDropped = 0;
std::atomic<unsigned>   numCached = 0;
std::atomic<unsigned>   numStorageFull = 0;
std::uint64_t testStartMs;

class MyDebugEventListener : public DebugEventListener {
public:
    bool print = false;
	virtual void OnDebugEvent(DebugEvent &evt)
	{
#ifdef USE_ECG
		int64_t cyclesPerTime = 0;
		{
			std::lock_guard<std::mutex> lock(dbg_callback_mtx);
			static int64_t prevCycles = 0;
			static int64_t currCycles = 0;
			static auto prevTime = std::chrono::steady_clock::now();
			static auto currTime = std::chrono::steady_clock::now();
			// Obtain current time
			currTime = std::chrono::steady_clock::now();
			currCycles = GetCPUCycles();
			// Calculate diff in milliseconds
			auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime).count();
			prevTime = currTime;
			if ((currCycles > prevCycles) && (((int64_t)diffTime) > 0)) {
				cyclesPerTime = (currCycles - prevCycles) / ((int64_t)diffTime);
				// Calculate CPU cycles per time
				auto currCycles = GetCPUCycles();
			}
			else {
				prevCycles = currCycles;
				return;
			}
			prevCycles = currCycles;
		}

		json j = {
			{ "evtSeq",      evt.seq },
			{ "memUse",      GetMemoryUsage() },
			{ "cpuCycles",   cyclesPerTime },
			{ "numThreads",  GetCurrentThreadCount() },
			{ "tcpCount",    GetCurrentTCPCount() },
			{ "udpCount",    GetCurrentUDPCount() },
		};
		switch (evt.type) {
		case EVT_SENT:
			j.push_back({ "evtSent", evt.param1 });
			break;
		case EVT_HTTP_OK:
			j.push_back({ "httpOK", evt.size });
			break;
		case EVT_HTTP_ERROR:
			j.push_back({ "httpERR", evt.size });
			break;
		}
		counterSink.log(j.dump());

#else

		// lock for the duration of the print, so that we don't mess up the prints
		std::lock_guard<std::mutex> lock(dbg_callback_mtx);
		std::uint64_t ms;

		switch (evt.type) {
		case EVT_LOG_EVENT:
		case EVT_LOG_LIFECYCLE:
		case EVT_LOG_FAILURE:
		case EVT_LOG_PAGEVIEW:
		case EVT_LOG_PAGEACTION:
		case EVT_LOG_SAMPLEMETR:
		case EVT_LOG_AGGRMETR:
		case EVT_LOG_TRACE:
		case EVT_LOG_USERSTATE:
		case EVT_LOG_SESSION:
			
			numLogged++;
            if (print)
            {
                printf("OnEventAdded:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numLogged._My_val, (unsigned int) evt.param2);
            }
			ms = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
			{
				uint64_t temp = (ms - testStartMs);
				if (temp > 0)
				{
					eps = (1000 * numLogged) / static_cast<unsigned int>(temp);
					if ((numLogged % 500) == 0)
					{
						printf("EPS=%u\n", eps._My_val);
					}
				}
			}
			break;
		case EVT_REJECTED:
            numRejected++;
			printf("OnEventRejected:    seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numRejected._My_val, (unsigned int)evt.param2);
			break;
		case EVT_ADDED:
            numLogged++;
            if(print)
			printf("OnEventAdded:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numLogged._My_val, (unsigned int) evt.param2);
			break;
		case EVT_CACHED:
			numCached += (unsigned int)evt.size;
            if(print)
			 printf("OnEventCached:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numCached._My_val, (unsigned int) evt.param2);
			break;
		case EVT_DROPPED:
            numDropped += (unsigned int)evt.size;
			printf("OnEventDropped:     seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numDropped._My_val, (unsigned int)evt.param2);
			break;
		case EVT_SENT:
			numSent += (unsigned int)evt.size;
            //if(print)
			printf("OnEventsSent:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numSent._My_val, (unsigned int) evt.param2);
			break;
		case EVT_STORAGE_FULL:
            numStorageFull++;
			printf("OnStorageFull:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, numStorageFull._My_val, (unsigned int)evt.param2);
			if (evt.param1 >= 75) {
				// UploadNow must NEVER EVER be called from Aria callback thread, so either use this structure below
				// or notify the main app that it has to do the profile timers housekeeping / force the upload...
				//std::thread([]() { LogManager::UploadNow(); }).detach();
			}
			break;
		case EVT_CONN_FAILURE:
		case EVT_HTTP_FAILURE:
		case EVT_COMPRESS_FAILED:
		case EVT_UNKNOWN_HOST:
		case EVT_SEND_FAILED:
			printf("OnEventsSendFailed: seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int) evt.param2);
			break;
		case EVT_HTTP_ERROR:
			printf("OnHttpError:        seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
				evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int)evt.param2, evt.data, (unsigned int)evt.size);
			break;
		case EVT_HTTP_OK:
            if(print)
			printf("OnHttpOK:           seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
				evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int) evt.param2, evt.data, (unsigned int)evt.size);
			break;
		case EVT_SEND_RETRY:
			printf("OnSendRetry:        seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
				evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int)evt.param2, evt.data, (unsigned int)evt.size);
			break;
		case EVT_SEND_RETRY_DROPPED:
			printf("OnSendRetryDropped: seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
				evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int) evt.param2, evt.data, (unsigned int)evt.size);
			break;
		case EVT_NET_CHANGED:
			printf("OnNetChanged:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u [%s]\n",
				evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int)evt.param2, networkCostNames[evt.param1]);
			if ((unsigned int) evt.param2)
			{
				printf("Malwarebytes Antiexploit has been detected! Network cost is unknown.\n");
			}
			break;
		case EVT_UNKNOWN:
		default:
            if (print)
			printf("OnEventUnknown:     seq=%llu, ts=%llu, type=0x%08x, Timerqueue=%u, ExecuteQueue=%u\n", evt.seq, evt.ts, evt.type, (unsigned int)evt.param1, (unsigned int)evt.param2);
			break;
		};
#endif
	};
};

MyDebugEventListener listener;

#define MAX_STRESS_COUNT            32
#define MAX_STRESS_THREADS          100

/// <summary>
/// New fluent syntax
/// </summary>
/// <param name="name"></param>
/// <param name="prio"></param>
/// <returns></returns>

// stress-test for a large string
#define MAX_WEIRDOS     8192
char weirdoBuffer[MAX_WEIRDOS] = { 'A' };

EventProperties CreateSampleEvent(const char *name, EventLatency prio) {

	GUID win_guid;
	win_guid.Data1 = 0;
	win_guid.Data2 = 1;
	win_guid.Data3 = 2;
	for (size_t i = 0; i < 8; i++)
	{
		win_guid.Data4[i] = static_cast<unsigned char>(i);
	}

	// GUID constructor from byte[16]
	const uint8_t guid_b[16] = {
		0x03, 0x02, 0x01, 0x00,
		0x05, 0x04,
		0x07, 0x06,
		0x08, 0x09,
		0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

	GUID_t guid_c(
		0x00010203,
		0x0405,
		0x0607,
		{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }
	);

	const GUID_t guid_d;

	// Prepare current time in UTC (seconds precision)
	std::time_t t = std::time(nullptr);
//	struct tm*   Tm;
	//std::gmtime_s(Tm,&t);

#if defined(_MSC_VER) && (_MSC_VER == 1800)
	/* map assignment operator for Visual Studio 2013, which may not fully support C++11 features. */
	std::map<std::string, EventProperty> values;
	values["_MSC_VER"] = _MSC_VER;

	values["piiKind.None"] = EventProperty("maxgolov", PiiKind_None);
	values["piiKind.DistinguishedName"] = EventProperty("/CN=Max Golovanov,OU=ARIA,DC=REDMOND,DC=COM", PiiKind_DistinguishedName);
	values["piiKind.GenericData"] = EventProperty("maxgolov", PiiKind_GenericData);
	values["piiKind.IPv4Address"] = EventProperty("127.0.0.1", PiiKind_IPv4Address);
	values["piiKind.IPv6Address"] = EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address);
	values["piiKind.MailSubject"] = EventProperty("RE: test", PiiKind_MailSubject);
	values["piiKind.PhoneNumber"] = EventProperty("+1-613-866-6960", PiiKind_PhoneNumber);
	values["piiKind.QueryString"] = EventProperty("a=1&b=2&c=3", PiiKind_QueryString);
	values["piiKind.SipAddress"] = EventProperty("sip:maxgolov@microsoft.com", PiiKind_SipAddress);
	values["piiKind.SmtpAddress"] = EventProperty("Max Golovanov <maxgolov@microsoft.com>", PiiKind_SmtpAddress);
	values["piiKind.Identity"] = EventProperty("Max Golovanov", PiiKind_Identity);
	values["piiKind.Uri"] = EventProperty("http://www.microsoft.com", PiiKind_Uri);
	values["piiKind.Fqdn"] = EventProperty("www.microsoft.com", PiiKind_Fqdn);

	values["strKey"] = "hello";
	values["strKey2"] = "hello2";
	values["int64Key"] = 1L;
	values["dblKey"] = 3.14;
	values["boolKey"] = false;
	values["guidKey0"] = GUID_t("00000000-0000-0000-0000-000000000000");
	values["guidKey1"] = GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F");
	values["guidKey2"] = GUID_t(guid_b);
	values["guidKey3"] = GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F");
	values["guidKey4"] = GUID_t(guid_c);
	values["timeKey1"] = time_ticks_t((uint64_t)0);
	values["timeKey2"] = time_ticks_t(&t);
	EventProperties props(name, values);
#else
	/* С++11 constructor for Visual Studio 2015: this is the most JSON-lookalike syntax that makes use of C++11 initializer lists. */
	EventProperties props(name,
	{
		//{ "MSC_VER", _MSC_VER },

		{ "piiKind.None",               EventProperty("maxgolov",  PiiKind_None) },
		{ "piiKind.DistinguishedName",  EventProperty("/CN=Max Golovanov,OU=ARIA,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
		{ "piiKind.GenericData",        EventProperty("maxgolov",  PiiKind_GenericData) },
		{ "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
		{ "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
		{ "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
		{ "piiKind.PhoneNumber",        EventProperty("+1-613-866-6960", PiiKind_PhoneNumber) },
		{ "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
		{ "piiKind.SipAddress",         EventProperty("sip:maxgolov@microsoft.com", PiiKind_SipAddress) },
		{ "piiKind.SmtpAddress",        EventProperty("Max Golovanov <maxgolov@microsoft.com>", PiiKind_SmtpAddress) },
		{ "piiKind.Identity",           EventProperty("Max Golovanov", PiiKind_Identity) },
		{ "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
		{ "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },

		{ "strKey",   "hello" },
		{ "strKey2",  "hello2" },
		{ "int64Key", 1L },
		{ "dblKey",   3.14 },
		{ "boolKey",  false },
		{ "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
		{ "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
		{ "guidKey2", GUID_t(guid_b) },
		{ "guidKey3", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
		{ "guidKey4", GUID_t(guid_c) },
		{ "timeKey1",  time_ticks_t((uint64_t)0) },     // ticks   precision
		{ "timeKey2",  time_ticks_t(&t) }               // seconds precision
        
	});
#endif
	props.SetProperty("win_guid", GUID_t(win_guid));
    props.SetProperty("Customer", "value", PiiKind::CustomerContentKind_GenericData);

	 GUID_t guidKey5("00000000-0000-0000-0000-000000000001");
//	 GUID_t &g = guidKey5;
	 props.SetProperty("refGuidKey5", guidKey5);

	props.SetLatency(prio);

#if 1 /* This may cause out of memory in a stress... */
	// This buffer is intentionally concurrently modified from different threads,
	// so it's random string of ASCII characeters pretty much, depending on how
	// many threads running and at what speed
	//for (size_t i = 0; i < MAX_WEIRDOS; i++)
	//{
	//	weirdoBuffer[i] = ' ' + (i % (127 - ' '));
	//}
	//props.SetProperty("weirdoString", (const char *)(&weirdoBuffer[0]));
#endif

	props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);

	return props;
}

void test_ProfileSwitch(ILogger *logger)
{
	printf("switching profile to Office_Telemetry_OneMinute\n");
	LogManager::SetTransmitProfile("Office_Telemetry_OneMinute");
	for (int i = 0; i < 10; i++)
	{
		std::string eventName = "eventName_5min_";
		eventName += std::to_string(i);
		logger->LogEvent(eventName);
	}
#if 0
	std::cout << "Press <ENTER> to switch to another profile..." << std::endl;
	fflush(stdout);
	fgetc(stdin);
#endif

	printf("switching profile to Office_Telemetry_TenSeconds\n");
	LogManager::SetTransmitProfile("Office_Telemetry_TenSeconds");
	for (int i = 0; i < 10; i++)
	{
		std::string eventName = "eventName_10sec_";
		eventName += std::to_string(i);
		logger->LogEvent(eventName);
	}
#if 0
	std::cout << "Press <ENTER> to continue..." << std::endl;
	fflush(stdout);
	fgetc(stdin);
#endif

}

void sendEmptyEvent(ILogger *logger)
{
	EventProperties props("test_empty_event");
	props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);
	logger->LogEvent(props);
}

ILogConfiguration& configuration = LogManager::GetLogConfiguration();
//ILogManager* lm;

ILogger* init() {
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH,"offlinestorage.db"); //":memory:"; //"offlinestorage.db";
	configuration.SetIntProperty("traceLevelMask",0xFFFFFFFF ^ 128); // API calls + Global mask for general messages - less SQL
													 //  configuration.minimumTraceLevel = ACTTraceLevel_Debug;
	configuration.SetMinimumTraceLevel(ACTTraceLevel_Trace);
	//configuration.multiTenantEnabled = true;
	configuration.SetIntProperty("cacheFileSizeLimitInBytes",150 * 1024 * 1024);
	configuration.SetIntProperty("cacheMemorySizeLimitInBytes", 50 * 1024 * 1024);
	configuration.SetIntProperty("maxTeardownUploadTimeInSec", 5);

    configuration.SetBoolProperty(CFG_BOOL_ENABLE_CRC32, true);
    configuration.SetBoolProperty(CFG_BOOL_ENABLE_HMAC, false);
    configuration.SetBoolProperty(CFG_BOOL_ENABLE_DB_COMPRESS, true);
    configuration.SetBoolProperty(CFG_BOOL_ENABLE_WAL_JOURNAL, false);
    configuration.SetIntProperty(CFG_INT_MAX_PKG_DROP_ON_FULL, 20);
    ACTStatus error;
    std::string temp = configuration.GetProperty("dsadasdsad", error);

	// Force UTC uploader on Windows 10 even if it's not RS2
	//configuration.SetSdkModeType( SdkModeTypes::SdkModeTypes_UTCAriaBackCompat);

#ifdef USE_INT
    configuration.SetProperty(CFG_STR_COLLECTOR_URL, "https://pipe.int.trafficmanager.net/OneCollector/1.0");//"https://pipe.dev.trafficmanager.net/OneCollector/1.0/"); //"https://pipe.int.trafficmanager.net/Collector/3.0/"); //"https://mobile.pipe.aria.microsoft.com/Collector/3.0/";// 
#endif

#ifdef USE_BOGUS_URL
	configuration.eventCollectorUri = "https://127.0.0.1/";
#endif

#if 0
	std::string leastCostProfile = R"(
[{
    "name": "LEAST_COST",
    "rules": [
    { "netCost": "restricted",                              "timers": [ -1, -1, -1 ] },
    { "netCost": "high",        "powerState": "battery",    "timers": [ -1, -1, -1 ] },
    { "netCost": "high",        "powerState": "charging",   "timers": [ 35, 17,  5 ] },
    { "netCost": "low",         "powerState": "battery",    "timers": [ 32, 16,  8 ] },
    { "netCost": "low",         "powerState": "charging",   "timers": [ 16,  8,  4 ] },
    { "netCost": "unknown",     "powerState": "battery",    "timers": [128, 64, 32 ] },
    { "netCost": "unknown",     "powerState": "charging",   "timers": [ 64, 32, 16 ] },
    {                                                       "timers": [100, 50, 17 ] }
    ]
}]
)";
#endif

    std::cout << "LogManager::Initialize..." << endl;

    std::cout << "LogManager::Initialize..." << endl;

    // Apply the profile before initialize
    LogManager::SetTransmitProfile("Office_Telemetry_TenSeconds");
    ILogger *result = LogManager::Initialize(cTenantToken);

	
	LogManager::AddEventListener(DebugEventType::EVT_LOG_EVENT, listener);
	LogManager::AddEventListener(DebugEventType::EVT_LOG_SESSION, listener);
	LogManager::AddEventListener(DebugEventType::EVT_REJECTED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_SEND_FAILED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_SENT, listener);
	LogManager::AddEventListener(DebugEventType::EVT_DROPPED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_HTTP_OK, listener);
	LogManager::AddEventListener(DebugEventType::EVT_HTTP_ERROR, listener);
	LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY, listener);
	LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY_DROPPED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_CACHED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_NET_CHANGED, listener);
	LogManager::AddEventListener(DebugEventType::EVT_STORAGE_FULL, listener);
    LogManager::AddEventListener(DebugEventType::EVT_UNKNOWN, listener);

	

	// TC for SetContext(<const char*,const char*, PiiKind>)
	const char* gc_value = "1234 :-)";
	LogManager::SetContext("GLOBAL_context", gc_value, PiiKind_MailSubject);

	return result;
}

std::mutex mtx_log_session;

void run(ILogger* logger, int maxStressRuns) {
	{

		for (int stressRuns = 0; stressRuns < maxStressRuns; stressRuns++)
		{
			bool doPause = false;
			bool doResume = false;

			if (doPause) {
				LogManager::PauseTransmission();
			}

			{
				// ignore the logger passed from above
				ILogger *loggerl = LogManager::GetLogger(cTenantToken); 

				// Set the custom context to be sent with every telemetry event.
                loggerl->SetContext("TeamName", "ARIA");
				//logger->SetContext("AppID", VER1 VER2 "-" __DATE__ " " __TIME__);
				// Set the semantic context. For example, an app will set this property after the user logs in.
				//logger->SetContext(->GetSemanticContext()->SetUserMsaId("BCCA864D-1386-4D5A-9570-B129F6DD42B7");
                loggerl->SetContext("context.string.key", "boo");

				long long_value = 12345L;
                loggerl->SetContext("context.long.key", long_value);

				double double_value = (double)((uint64_t)9223372036854775807L);
                loggerl->SetContext("context.double.key", double_value);

				{
					EventProperties props = CreateSampleEvent("Sample.Event.Low", EventLatency_Normal);
                    loggerl->LogEvent(props);
				}

				if ((stressRuns % 2) == 0)
				{
					EventProperties props = CreateSampleEvent("Sample.Event.Normal", EventLatency_CostDeferred);
                    loggerl->LogEvent(props);
				}

				if ((stressRuns % 4) == 0)
				{
					EventProperties props = CreateSampleEvent("Sample.Event.High", EventLatency_RealTime);
					props.SetType("My.Super.Duper.Fancy.Event.Type.For.MDM.Export");
                    loggerl->LogEvent(props);
				}

				if ((stressRuns % 8) == 0)
				{
                    EventProperties props = CreateSampleEvent("Sample.Event.Immediate", EventLatency_RealTime);// EventPriority_Immediate);                    
                    loggerl->LogEvent(props);
				}
			}
						

			// Send empty EventProperties
			sendEmptyEvent(logger);

			{
				// LogSession API is not thread-safe by design -- use logger passed from above
				std::lock_guard<std::mutex> lock(mtx_log_session);
				EventProperties props("LogSessionTest");
				props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);
				props.SetPriority(EventPriority_High);
				logger->LogSession(SessionState::Session_Started, props);
				logger->LogSession(SessionState::Session_Ended, props);
			}

			if (doResume) {
				LogManager::ResumeTransmission();
			}

#ifdef _RANDOM_DELAY_AFTER_LOG
			// Let event at least try to reach the server
			srand(time(NULL));
			unsigned sleepTime = 1000 * (rand() % 7);
			std::cout << "sleep " << sleepTime << std::endl;
			_sleep(sleepTime);
#endif

		//std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
        
		//LogManager::UploadNow();
	}
}

void test_failure(ILogger *logger) {
	try {
		// This event should be rejected
		logger->LogEvent("!!!EPIC FAIL!!!");
	}
	catch (...) {

	}
}

void done() {
	std::cout << "LogManager::FlushAndTeardown()..." << std::endl;
	LogManager::FlushAndTeardown();
}

void DumpMemoryLeaks()
{
	printf("Terminating... (waiting for threads to finish before _CrtDumpMemoryLeaks)\n");
	std::thread([] {
		// Let all other atexit handlers to finish
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		_CrtDumpMemoryLeaks();
	}).detach();
}

int main(int argc, char* argv[])
{//
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
#ifdef DETECT_MEMLEAKS
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_EVERY_1024_DF);
	std::atexit(DumpMemoryLeaks);
#endif

    DWORD start = GetTickCount();
	LogManager::LoadTransmitProfiles(transmitProfileDefinitions);
	LogManager::SetTransmitProfile("Office_Telemetry_OneMinute");

	std::vector<std::thread> workers;
	std::thread t[MAX_STRESS_THREADS];

	ILogger* logger = init();
	ILogger* logger2 = LogManager::GetLogger(TOKEN2, "tenant2");

	{
		EventProperties props = CreateSampleEvent("Sample.Event.Low", EventLatency_Normal);
		logger->LogEvent(props);
    }


	printf("test_ProfileSwitch\n");
	test_ProfileSwitch(logger);

    EventProperties props = CreateSampleEvent("Sample.Event.Immediate", EventLatency_Max);
    logger->LogEvent(props);

	// Run multi-threaded stress for multi-tenant
	testStartMs = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	for (int i = 0; i < MAX_STRESS_THREADS; i++) {
		workers.push_back(std::thread([i, logger, logger2]()
		{
			std::string threadName = "test_thread_";
			threadName += std::to_string(i);
			SetThreadName(threadName.c_str());
			run(logger, MAX_STRESS_COUNT);
			run(logger2, MAX_STRESS_COUNT);
		}));
	}
/*
	std::this_thread::sleep_for(std::chrono::milliseconds(90000));

	for (int i = 0; i < MAX_STRESS_THREADS; i++) {
		workers.push_back(std::thread([i, logger, logger2]()
		{
			std::string threadName = "test_thread_";
			threadName += std::to_string(i);
			SetThreadName(threadName.c_str());
			run(logger, MAX_STRESS_COUNT);
			run(logger2, MAX_STRESS_COUNT);
		}));
	}
*/
	// Wait for completion of all worker threads
	std::for_each(workers.begin(), workers.end(), [](std::thread &t)
	{
		if (t.joinable())
		{
			t.join();
		}
	});

	//LogManager::UploadNow();
	// save to disk
	LogManager::Flush();

//all_done:
    	    
    {
        EventProperties lprops = CreateSampleEvent("Sample.Event.Low", EventLatency_Normal);
        logger->LogEvent(lprops);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    //listener.print = true;
	// Flush and Teardown
	done();
    

    bool waitForUser = true;
    if (waitForUser) {
        std::cout << "Press <ENTER> to FlushAndTeardown" << std::endl;
        fflush(stdout);
        fgetc(stdin);
    }


    listener.print = false;
//	delete lm;
/*	// 2nd run after initialize
	{		
		printf("Reinitialize test...\n");
        ILogger* logger = init();
		
		std::map<std::string, ILogger*> loggers;
		//loggers["logger.noparam"] = LogManager::GetLogger();
		//loggers["logger.blank"] = LogManager::GetLogger("");
		//loggers["logger.blank.s1"] = LogManager::GetLogger("", "s1");
		//loggers["logger.blank.s2"] = LogManager::GetLogger("", "s2");
		//loggers["logger.invalid"] = LogManager::GetLogger("s3", "12345");
		//loggers["logger.invalid"] = LogManager::GetLogger("12345", "s3");
		loggers["logger.primary"] = LogManager::Initialize(TOKEN, configuration);
		loggers["logger.t1s1"] = LogManager::GetLogger(TOKEN, "s1");
		loggers["logger.t1s2"] = LogManager::GetLogger(TOKEN, "s2");
		loggers["logger.t2s1"] = LogManager::GetLogger(TOKEN2, "s1");
		loggers["logger.t2s2"] = LogManager::GetLogger(TOKEN2, "s2");
		for (auto &kv : loggers)
		{
			for (size_t i = 0; i < MAX_STRESS_COUNT; i++)
			{
				ILogger *loggerl = kv.second;
				EventProperties props("traceEventName",
				{
					{ "logger"  , kv.first.c_str() }
				});
				props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);
                loggerl->LogTrace(TraceLevel_Error, "some error occurred", props);
			}
		};
		LogManager::UploadNow();

        listener.print = true;
        {
            EventProperties props = CreateSampleEvent("Sample.Event.Low", EventPriority_Low);
            loggers["logger.primary"]->LogEvent(props);
        }
		done();
		delete lm;
	}
*/

#ifdef DETECT_MEMLEAKS
	_CrtDumpMemoryLeaks();
#endif
	fgetc(stdin);
    DWORD end = GetTickCount();
    printf("Time Taken to run this all= %d test...\n", end - start );
   
    printf("numLogged:      p1=%u, \n", numLogged._My_val);
    printf("numRejected:    p1=%u\n", numRejected._My_val);
    printf("numCached:      p1=%u\n", numCached._My_val);
    printf("numDropped:     p1=%u\n", numDropped._My_val);
    printf("numSent:        p1=%u\n", numSent._My_val);
    printf("numStorageFull: p1=%u\n", numStorageFull._My_val);
    
	return 0;
}

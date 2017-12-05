#pragma managed(push, off)

// user and computer name
#include <windows.h>
#include <Lmcons.h>
#include <locale>
#include <codecvt>

// for local time
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sstream>

#include "NativeLogger.h"

#include "api/ILogManagerInternal.hpp"
#include "LogManager.hpp"
#include "Enums.hpp"
#include "CorrelationVector.hpp"

using namespace Microsoft::Applications::Telemetry;
using namespace std;

NativeLogger::NativeLogger() : m_pLogger(nullptr), m_eventCount(0)
{
}

NativeLogger::~NativeLogger()
{
	// Windows Desktop-specific: call this method upon the application exit.
	LogManager::FlushAndTeardown();
}

void NativeLogger::Start(bool useUtc)
{
	m_pLogger = CreateLogger(useUtc);
}

void NativeLogger::Stop()
{

}

void NativeLogger::LogEvents(int count, bool isCritical, bool isRealtime)
{
	if (!m_pLogger)
	{
		return;
	}

	// ucs2 to utf8 converter
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	// collect user name
	TCHAR username[UNLEN + 1];
	DWORD usernameLength = UNLEN + 1;
	GetUserName(username, &usernameLength);

	// collect computer name
	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD computerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
	GetUserName(computerName, &computerNameLength);

	// get local time
	tm locTime;
	time_t t = time(nullptr);
	localtime_s(&locTime, &t);
	std::ostringstream oss;
	oss << (locTime.tm_year + 1900) << "-"
		<< locTime.tm_mon << "-"
		<< locTime.tm_mday << " "
		<< locTime.tm_hour << ":"
		<< locTime.tm_min << ":"
		<< locTime.tm_sec;
	string localTime = oss.str();

	uint64_t flags = (isCritical ? MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_KEYWORD_CRITICAL_DATA : 0)
                   | (isRealtime ? MICROSOFT_EVENTTAG_REALTIME_LATENCY : 0);

    // Construct and initialize a correlation vector with a random base value,
    // share that value across the app components which are going to use it.
    // There could be, for example, one CV per app, per scenario, per user.
    CorrelationVector m_appCV;
    m_appCV.Initialize(2);

	for (int i = 0; i < count; i++)
	{
		EventProperties eventData("Microsoft.OneSDK.Example.HelloWorldEvent");

        eventData.SetProperty(CorrelationVector::PropertyName, m_appCV.GetNextValue());
		eventData.SetProperty("DeveloperName", converter.to_bytes(username), PiiKind_Identity);
		eventData.SetProperty("ComputerName", converter.to_bytes(computerName), PiiKind_Identity);
		eventData.SetProperty("LocalTime", localTime);
		eventData.SetProperty("EventCount", ++m_eventCount);
		eventData.SetPolicyBitFlags(flags);

		((ILogger *)m_pLogger)->LogEvent(eventData);
	}
}

void * NativeLogger::CreateLogger(bool useUtc)
{
	// Windows SDK Test - Int: Default Ingestion Token.
	std::string tenantToken = "0c21c15bdccc48c99678a748488bb87f-cca6848e-b4aa-48a6-b24a-0170caf27523-7582";

	ILogConfiguration& configuration = LogManager::GetLogConfiguration();
	configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, "offlinestorage.db"); //":memory:"; //"offlinestorage.db";
	configuration.SetIntProperty("traceLevelMask", 0xFFFFFFFF ^ 128); // API calls + Global mask for general messages - less SQL
													 //  configuration.minimumTraceLevel = ACTTraceLevel_Debug;
	configuration.SetMinimumTraceLevel(ACTTraceLevel_Trace);
	//configuration.multiTenantEnabled = true;
	configuration.SetIntProperty("cacheFileSizeLimitInBytes", 150 * 1024 * 1024);
	configuration.SetIntProperty("cacheMemorySizeLimitInBytes", 50 * 1024 * 1024);
	configuration.SetIntProperty("maxTeardownUploadTimeInSec", 5);

	configuration.SetProperty(CFG_BOOL_ENABLE_CRC32, "true");
	configuration.SetProperty(CFG_BOOL_ENABLE_HMAC, "false");
	configuration.SetProperty(CFG_BOOL_ENABLE_DB_COMPRESS, "true");
	configuration.SetProperty(CFG_BOOL_ENABLE_WAL_JOURNAL, "false");
	configuration.SetProperty(CFG_INT_MAX_PKG_DROP_ON_FULL, "20");
//  std::string temp = configuration.GetProperty("dsadasdsad");

	configuration.SetSdkModeType(useUtc ? SdkModeTypes::SdkModeTypes_UTCAriaBackCompat : SdkModeTypes::SdkModeTypes_Aria);

	//"https://pipe.dev.trafficmanager.net/OneCollector/1.0/");
	//"https://pipe.int.trafficmanager.net/Collector/3.0/");
	//"https://mobile.pipe.aria.microsoft.com/Collector/3.0/";

	configuration.SetProperty(CFG_STR_COLLECTOR_URL, "https://pipe.int.trafficmanager.net/OneCollector/1.0");

	// Apply the profile before initialize
	LogManager::SetTransmitProfile("Office_Telemetry_TenSeconds");
	ILogger *result = LogManager::Initialize(tenantToken);

	// TC for SetContext(<const char*,const char*, PiiKind>)
	const char* gc_value = "1234 :-)";
	LogManager::SetContext("GLOBAL_context", gc_value, PiiKind_MailSubject);

	return result;
}

#pragma managed(pop)
#pragma once
#include "Version.hpp"
#include "IHttpClient.hpp"
#include "IRuntimeConfig.hpp"
#include "IBandwidthController.hpp"

#ifdef ARIASDK_PAL_SKYPE
    #include <httpstack/fwd.hpp>
    #include <ecsClientInterface.hpp>
    #include <ResourceManager/ResourceManagerPublic.hpp>
#endif

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*

	static const char* COLLECTOR_URL_UNITED_STATES = "https://us.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_GERMANY = "https://de.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_AUSTRALIA = "https://au.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_JAPAN = "https://jp.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_EUROPE = "https://eu.pipe.aria.microsoft.com/Collector/3.0/";

	enum SdkModeTypes
	{
		SdkModeTypes_Aria = 0, //This is default transmission mode
		SdkModeTypes_UTCAriaBackCompat = 1,
		SdkModeTypes_UTCCommonSchema = 2
	};

struct LogConfiguration
{
    /// [optional] Url of the collector for sending events
    /// default will be used if not specified
    std::string eventCollectorUri;

    /// [optional] Url of the collector for sending BLOB data
    /// default will be used if not specified
    std::string blobCollectorUri;

    /// [optional] Full path of the disk file used to cache events on client side
    /// Specify one to avoid/reduce potential loss of data by persisting events
    /// to file storage for them to be sent in next telemetry session.
    std::string cacheFilePath;

    /// [optional] Size limit of the disk file used to cache events on client side
    /// Additional events might cause older events in the file cache to be dropped
    /// This size limit should be larger than the cacheMemorySizeLimitInBytes below
    unsigned int cacheFileSizeLimitInBytes = 0;

    /// [optional] Memory size limit that allows events to be cached in memory
    /// Additional events will cause older events to be flushed to disk file
    unsigned int cacheMemorySizeLimitInBytes = 0;

    /// [optional] Allows overriding tenant token for all logged events.
    /// Useful when testing reliability, amount of events sent by an app etc.
    std::string forcedTenantToken;

    /// [optional] Pointer to HTTP client object
    /// Target object must be kept alive for the lifetime of ILogManager.
    IHttpClient* httpClient = nullptr;

    /// [optional] Pointer to runtime configuration provider object
    /// Target object must be kept alive for the lifetime of ILogManager.
    IRuntimeConfig* runtimeConfig = nullptr;

    /// [optional] Pointer to bandwidth controller object
    /// Target object must be kept alive for the lifetime of ILogManager.
    IBandwidthController* bandwidthController = nullptr;

#ifdef ARIASDK_PAL_SKYPE
    /// [optional] Pointer to Skype HTTP Stack
    /// The library will add and hold a reference to the target object.
    ::http_stack::IHttpStack * skypeHttpStack = nullptr;

    /// [optional] Pointer to Skype ECS Client
    /// Target object must be kept alive for the lifetime of ILogManager.
    ::ecsclient::IEcsClient* skypeEcsClient = nullptr;

    /// [optional] Smart pointer to Skype Resource Manager
    /// The library will add and hold a reference to the target object.
    ::resource_manager2::ResourceManagerPtr skypeResourceManager;
#endif

    /// Controls whether the SDK should call sqlite3_initialize() and
    /// sqlite3_shutdown() itself. Set to true if the app or other library
    /// is already doing that.
    bool skipSqliteInitAndShutdown = false;


	/// <summary>[optional] Url of the collector for sending events.
	/// default will be false
	/// </summary>
	bool enableLifecycleSession = false;


	/// <summary>[optional] Enable multiTenant
	/// default will be true
	/// </summary>
	bool multiTenantEnabled;

	/// <summary>[optional] Debug trace module mask controls what modules may emit debug output.<br>
	/// default is UINT_MAX - monitor all modules</summary>
	unsigned int  traceLevelMask;

	/// <summary>[optional] Debug trace level mask controls global verbosity level.<br>
	/// default is ACTTraceLevel_Error</summary>
	ACTTraceLevel minimumTraceLevel;

	/// <summary>Api to set Aria SDK mode with Non UTC, UTC with common Schema or UTC with Aria Schema.<br>
	/// default is Non UTC</summary>
	SdkModeTypes sdkmode;

	/// <summary>[optional] Maximum amount of time (in seconds) allotted to upload in-ram and offline records on teardown.<br>
	/// If device is in a state where events are not allowed to be transmitted (offline, roaming, etc.), then the value is ignored.<br>
	/// default duration is 0</summary>
	unsigned int maxTeardownUploadTimeInSec;

	///<summary>LogConfiguration constructor</summary>
	LogConfiguration():sdkmode(SdkModeTypes::SdkModeTypes_Aria), multiTenantEnabled(true), maxTeardownUploadTimeInSec(0)
	{
		// Apply debug trace level to all ARIA SDK modules by default
		traceLevelMask = UINT_MAX;
		minimumTraceLevel = ACTTraceLevel_Error;
	}

};


}}} // namespace Microsoft::Applications::Telemetry

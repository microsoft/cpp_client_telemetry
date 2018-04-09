#include "pch.h"

#include "LogConfigurationCX.hpp"
#include "LogManagerCX.hpp"

#include "PlatformHelpers.h"

#include "LogManager.hpp"

namespace MATW_NS_BEGIN {

    void LogConfiguration::ToLogConfigurationCore()
    {
        MAT::ILogConfiguration& configurationCore = MAT::LogManager::GetLogConfiguration();
        configurationCore[MAT::CFG_INT_SDK_MODE] = (int)this->SdkMode;
        // FIXME: [MG] - this assignment operator has to copy the source string rather than const char * ptr
        configurationCore[MAT::CFG_STR_CACHE_FILE_PATH]   = FromPlatformString(this->OfflineStorage).c_str();
        configurationCore[MAT::CFG_STR_COLLECTOR_URL]     = FromPlatformString(this->CollectorURL).c_str();
        configurationCore[MAT::CFG_INT_TRACE_LEVEL_MASK]  = this->TraceLevelMask;
        configurationCore[MAT::CFG_INT_TRACE_LEVEL_MIN]   = (int)(this->MinTraceLevel);
        configurationCore[MAT::CFG_INT_MAX_TEARDOWN_TIME] = this->MaxTeardownUploadTimeInSec;
        configurationCore[MAT::CFG_INT_MAX_PENDING_REQ]   = this->MaxPendingHTTPRequests;
        configurationCore[MAT::CFG_INT_RAM_QUEUE_BUFFERS] = this->MaxDBFlushQueues;
    }

} MATW_NS_END


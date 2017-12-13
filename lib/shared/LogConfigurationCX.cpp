#include "pch.h"

#include "LogConfigurationCX.hpp"
#include "LogManagerCX.hpp"

#include "PlatformHelpers.h"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                void LogConfiguration::ToLogConfigurationCore()
                {
                    MAT::ILogConfiguration& configurationCore = MAT::LogManager::GetLogConfiguration();
                    configurationCore.SetSdkModeType((MAT::SdkModeTypes)this->SdkMode);
                    configurationCore.SetProperty(CFG_STR_CACHE_FILE_PATH, FromPlatformString(this->OfflineStorage).c_str());
                    configurationCore.SetProperty(CFG_STR_COLLECTOR_URL, FromPlatformString(this->CollectorURL).c_str());
                    configurationCore.SetIntProperty(CFG_INT_TRACE_LEVEL_MASK, this->TraceLevelMask);
                    configurationCore.SetMinimumTraceLevel((MAT::ACTTraceLevel)(this->MinTraceLevel));
                    configurationCore.SetIntProperty(CFG_INT_MAX_TEARDOWN_TIME, this->MaxTeardownUploadTimeInSec);
                    configurationCore.SetIntProperty(CFG_INT_MAX_PENDING_REQ, this->MaxPendingHTTPRequests);
                    configurationCore.SetIntProperty(CFG_INT_RAM_QUEUE_BUFFERS, this->MaxDBFlushQueues);
                }
            }
        }
    }
}

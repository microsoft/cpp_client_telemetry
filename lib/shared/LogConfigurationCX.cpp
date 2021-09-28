//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
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
        configurationCore[MAT::CFG_STR_CACHE_FILE_PATH]         = FromPlatformString(this->OfflineStorage);
        configurationCore[MAT::CFG_INT_CACHE_FILE_SIZE]         = this->CacheFileSizeLimitInBytes;
        configurationCore[MAT::CFG_BOOL_ENABLE_DB_DROP_IF_FULL] = this->EnableDBDropIfFull;
        configurationCore[MAT::CFG_STR_COLLECTOR_URL]           = FromPlatformString(this->CollectorURL);
        configurationCore[MAT::CFG_INT_TRACE_LEVEL_MASK]        = this->TraceLevelMask;
        configurationCore[MAT::CFG_INT_TRACE_LEVEL_MIN]         = (int)(this->MinTraceLevel);
        configurationCore[MAT::CFG_INT_MAX_TEARDOWN_TIME]       = this->MaxTeardownUploadTimeInSec;
        configurationCore[MAT::CFG_INT_MAX_PENDING_REQ]         = this->MaxPendingHTTPRequests;
        configurationCore[MAT::CFG_INT_RAM_QUEUE_BUFFERS]       = this->MaxDBFlushQueues;

        // Populate these two fields at C++ core only if not empty at C# projection
        if (!this->StartProfileName->Equals(""))
            configurationCore[MAT::CFG_STR_START_PROFILE_NAME] = FromPlatformString(this->StartProfileName);
        if (!this->TransmitProfiles->Equals(""))
            configurationCore[MAT::CFG_STR_TRANSMIT_PROFILES]  = FromPlatformString(this->TransmitProfiles);
    }

} MATW_NS_END



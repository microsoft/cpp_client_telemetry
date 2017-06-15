#define LOG_MODULE DBG_API

#pragma unmanaged

#include "ECSClientUtils.hpp"

#include "common/Misc.hpp"
#include "common/TraceHelper.hpp"

#include <assert.h>
#include <string>
#include <stdexcept>

#ifdef _WINRT_DLL
#pragma message ("Compiling ECS client WINRT SDK...")
#include "Windows.h"
#include "PlatformHelpers.h"
using namespace ::Windows::Storage;
#else
#pragma message ("Compiling ECS client classic SDK...")
#endif

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace ECS {

                std::string CreateServerUrl(
                    const std::string& serverUrl,
                    const std::string& clientName,
                    const std::string& clientVersion)
                {
                    assert(!serverUrl.empty());
                    assert(!clientName.empty());

                    std::string resultUrl = common::Combine(serverUrl, clientName, '/');

                    std::string clientVer = common::ConvertClientVersion(clientVersion);
                    if (!clientVer.empty())
                    {
                        resultUrl = common::Combine(resultUrl, clientVer, '/');
                    }

                    return resultUrl;
                }

                void ThrowECSError(ECS_ERROR errCode, const char* pFile, int lineNumber)
                {
                    std::string errMsg;

                    switch (errCode)
                    {
                    case ECS_ERROR_INVALID_CONFIG:
                        errMsg = "Invalid ECSClientConfiguration specified";
                        LOG_ERROR("[ECSClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::invalid_argument(errMsg));
                        break;

                    case ECS_ERROR_OUT_OF_MEMORY:
                        errMsg = "Out of momery";
                        LOG_ERROR("[ECSClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::bad_alloc());
                        break;

                    case ECS_ERROR_INVALID_STATUS:
                        errMsg = "This operation is not allowed at current state";
                        LOG_ERROR("[ECSClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::logic_error(errMsg));
                        break;

                    default:
                        errMsg = "Runtime error occurred.";
                        LOG_ERROR("[ECSClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::runtime_error(errMsg));
                        break;
                    }
                }

                std::string GetStoragePath() {
#ifdef _WINRT_DLL
                    StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
                    Platform::String^ storagePath = localFolder->Path->ToString();
                    return Microsoft::Applications::Telemetry::Windows::FromPlatformString(storagePath).c_str();
#else
                    return "";
#endif
                }

            }
        }
    }
}
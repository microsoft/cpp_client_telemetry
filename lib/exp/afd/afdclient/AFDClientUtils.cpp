#define LOG_MODULE DBG_API

#pragma unmanaged

#include "AFDClientUtils.hpp"

#include "common/Misc.hpp"
#include "common/TraceHelper.hpp"

#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
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
            namespace AFD {

                 void ThrowAFDError(AFD_ERROR errCode, const char* pFile, int lineNumber)
                {
                    std::string errMsg;

                    switch (errCode)
                    {
                    case AFD_ERROR_INVALID_CONFIG:
                        errMsg = "Invalid AFDClientConfiguration specified";
                        LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::invalid_argument(errMsg));
                        break;

                    case AFD_ERROR_OUT_OF_MEMORY:
                        errMsg = "Out of momery";
                        LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::bad_alloc());
                        break;

                    case AFD_ERROR_INVALID_STATUS:
                        errMsg = "This operation is not allowed at current state";
                        LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
                        THROW(std::logic_error(errMsg));
                        break;

                    default:
                        errMsg = "Runtime error occurred.";
                        LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
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
				            

                std::vector<std::string> splitString(std::string str, char delimiter)
                {
                    std::vector<std::string> internal;
                    std::stringstream ss(str); 
                    std::string token;

                    while (getline(ss, token, delimiter))
                    {
                        internal.push_back(token);
                    }

                    return internal;
                }
            }
        }
    }
}
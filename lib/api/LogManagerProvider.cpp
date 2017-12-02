#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "LogManagerProvider.hpp"
#include "IHostLogManager.hpp"
#include "IGuestLogManager .hpp"
#include "HostGuestLogManager.hpp"
#include "LogSessionData.hpp"
#include "LogManagerImpl.hpp"

#include "TransmitProfiles.hpp"


#include <atomic>
#include <map>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            struct hostLogManagerHolder
            {
                int refcount = 0;
                HostGuestLogManager* hostLogManager;
            };

            std::mutex*                                 our_LogManagerProviderlockP = new std::mutex();
            std::map<std::string, hostLogManagerHolder> our_LogManagers;
            std::vector<LogConfiguration*>              our_LogConfigurations;

            ILogConfiguration& LogManagerProvider::CreateLogConfiguration()
            {
                ARIASDK_LOG_DETAIL("FlushAndTeardown()");
                LogConfiguration* temp = new LogConfiguration();
                our_LogConfigurations.push_back(temp);
                return *temp;
            }

            IHostLogManager* LogManagerProvider::GetHostLogManager(ILogConfiguration& logConfiguration,
                                                                   char const* apiKey,
                                                                   ACTStatus& status,
                                                                   uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[2]: apiKey=%s, configuration=0x%X", apiKey, logConfiguration);

                if (!apiKey)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                    return nullptr;
                }
                LogConfiguration* config = dynamic_cast<LogConfiguration*>(&logConfiguration);

                if (CurrentTargetVersion != targetVersion)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                }
                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) == our_LogManagers.end())
                    {
                        hostLogManagerHolder temp;
                        temp.hostLogManager = new HostGuestLogManager(config);
                        temp.refcount = 1;
                        our_LogManagers[apiKey] = temp;
                    }
                    else
                    {
                        our_LogManagers[apiKey].refcount++;
                    }
                }
                status = ACTStatus::ACTStatus_OK;
                return our_LogManagers[apiKey].hostLogManager;
            }

            IHostLogManager* LogManagerProvider::GetHostLogManager(char const* apiKey, ACTStatus& status, uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[1]: apiKey=%s", apiKey);
                if (!apiKey)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                    return nullptr;
                }
                if (CurrentTargetVersion != targetVersion)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                }

                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) == our_LogManagers.end())
                    {
                        hostLogManagerHolder temp;
                        temp.hostLogManager = new HostGuestLogManager(nullptr);
                        temp.refcount = 1;
                        our_LogManagers[apiKey] = temp;
                    }
                    else
                    {
                        our_LogManagers[apiKey].refcount++;
                    }
                }
                
                status = ACTStatus::ACTStatus_OK;
                return our_LogManagers[apiKey].hostLogManager;
            }

            IGuestLogManager* LogManagerProvider::GetGuestLogManager(char const* apiKey, ACTStatus& status, uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[1]: apiKey=%s", apiKey);
                if (!apiKey)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                    return nullptr;
                }
                if (CurrentTargetVersion != targetVersion)
                {
                    status = ACTStatus::ACTStatus_NotSupported;
                }

                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) == our_LogManagers.end())
                    {
                        hostLogManagerHolder temp;
                        temp.hostLogManager = new HostGuestLogManager(nullptr);
                        temp.refcount = 1;
                        our_LogManagers[apiKey] = temp;
                    }
                    else
                    {
                        our_LogManagers[apiKey].refcount++;
                    }
                }
                
                //IGuestLogManager* temp = dynamic_cast<IGuestLogManager*>(our_LogManagers[apiKey].hostLogManager);
                status = ACTStatus::ACTStatus_OK;
                return our_LogManagers[apiKey].hostLogManager;
            }   

            ACTStatus LogManagerProvider::DestroyLogManager(char const* apiKey)
            {
                if (!apiKey)
                {
                    return ACTStatus::ACTStatus_NotSupported;
                }
                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) != our_LogManagers.end())
                    {
                        our_LogManagers[apiKey].refcount--;
                        if (our_LogManagers[apiKey].refcount == 0)
                        {
                            delete our_LogManagers[apiKey].hostLogManager;
                            our_LogManagers.erase(apiKey);
                        }
                    }
                }
                return ACTStatus::ACTStatus_OK;
            }
        }
    }
}
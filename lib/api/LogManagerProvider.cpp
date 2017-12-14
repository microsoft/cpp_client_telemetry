#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "LogManagerProvider.hpp"
#include "ILogManager.hpp"
#include "HostGuestLogManager.hpp"


#include <atomic>
#include <map>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Events  {
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

            ILogManager* LogManagerProvider::CreateLogManager(char const* apiKey,
                                                              bool wantController,
                                                              ILogConfiguration& logConfiguration,
                                                              EVTStatus& status,
                                                              uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[2]: apiKey=%s, configuration=0x%X", apiKey, logConfiguration);

                if (!apiKey)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                    return nullptr;
                }
                if (CurrentTargetVersion != targetVersion)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                }

                LogConfiguration* config = dynamic_cast<LogConfiguration*>(&logConfiguration);
                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) == our_LogManagers.end())
                    {
                        hostLogManagerHolder temp;
                        temp.hostLogManager = new HostGuestLogManager(config, wantController);
                        temp.refcount = 1;
                        our_LogManagers[apiKey] = temp;
                    }
                    else
                    {
                        our_LogManagers[apiKey].refcount++;
                    }
                }
                status = EVTStatus::EVTStatus_OK;
                return our_LogManagers[apiKey].hostLogManager;
            }

            ILogManager* LogManagerProvider::CreateLogManager(char const* apiKey, bool wantController, EVTStatus& status, uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[1]: apiKey=%s", apiKey);
                if (!apiKey)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                    return nullptr;
                }
                if (CurrentTargetVersion != targetVersion)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                    return nullptr;
                }

                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) == our_LogManagers.end())
                    {
                        hostLogManagerHolder temp;
                        temp.hostLogManager = new HostGuestLogManager(nullptr, wantController);
                        temp.refcount = 1;
                        our_LogManagers[apiKey] = temp;
                    }
                    else
                    {
                        our_LogManagers[apiKey].refcount++;
                    }
                }
                
                status = EVTStatus::EVTStatus_OK;
                return our_LogManagers[apiKey].hostLogManager;
            }

            ILogManager* LogManagerProvider::CreateLogManager(char const* apiKey, EVTStatus& status, uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[1]: apiKey=%s", apiKey);
                return LogManagerProvider::CreateLogManager(apiKey, false, status, targetVersion);
            }   

            ILogManager* LogManagerProvider::GetLogManager(char const* apiKey, EVTStatus& status, uint32_t targetVersion)
            {
                ARIASDK_LOG_DETAIL("Initialize[1]: apiKey=%s", apiKey);
                if (!apiKey)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                    return nullptr;
                }
                if (CurrentTargetVersion != targetVersion)
                {
                    status = EVTStatus::EVTStatus_NotSupported;
                    return nullptr;
                }

                {
                    std::lock_guard<std::mutex> lock(*our_LogManagerProviderlockP);
                    if (our_LogManagers.find(apiKey) != our_LogManagers.end())
                    {
                        status = EVTStatus::EVTStatus_OK;
                        return our_LogManagers[apiKey].hostLogManager;
                    }
                }
                status = EVTStatus::EVTStatus_Fail;

                return nullptr;
            }

            EVTStatus LogManagerProvider::DestroyLogManager(char const* apiKey)
            {
                if (!apiKey)
                {
                    return EVTStatus::EVTStatus_NotSupported;
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
                return EVTStatus::EVTStatus_OK;
            }
        }
    }
}
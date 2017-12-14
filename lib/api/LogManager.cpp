#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "LogManager.hpp"
#include "LogManagerProvider.hpp"
#include "HostGuestLogManager.hpp"
#include "LogConfiguration.hpp"
#include "TransmitProfiles.hpp"


#include <atomic>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Events  {

			std::mutex*          our_lockP = new std::mutex();
			ILogManager*         our_pLogManagerSingletonInstanceP = nullptr;
            LogConfiguration*    our_LogConfigurationP = new LogConfiguration();
            ILogController*      our_LogController = nullptr;
            IAuthTokensController* our_AuthTokenController = nullptr;

            IAuthTokensController* LogManager::GetAuthTokenController()
            {
                return our_AuthTokenController;
            }

			ILogger* LogManager::Initialize(const std::string& tenantToken)
			{
				ARIASDK_LOG_DETAIL("Initialize[2]: tenantToken=%s, configuration=0x%X", tenantToken.c_str(), our_LogConfigurationP);
				std::lock_guard<std::mutex> lock(*our_lockP);

				if (nullptr == our_pLogManagerSingletonInstanceP)
				{
                    EVTStatus error;
                    std::string cacheFilePath = our_LogConfigurationP->GetProperty(CFG_STR_CACHE_FILE_PATH, error);
                    if (cacheFilePath.empty())
                    {
                        our_LogConfigurationP->SetProperty(CFG_STR_CACHE_FILE_PATH, tenantToken.c_str());
                    }
					our_pLogManagerSingletonInstanceP = LogManagerProvider::CreateLogManager("OldInterface",true,*our_LogConfigurationP, error);
                    our_LogController = our_pLogManagerSingletonInstanceP->GetLogController();
                    our_AuthTokenController = our_pLogManagerSingletonInstanceP->GetAuthTokensController();
				}

				ILogger *result = our_pLogManagerSingletonInstanceP->GetLogger(tenantToken);

                switch (our_LogConfigurationP->GetMinimumTraceLevel())
                {
                case    ACTTraceLevel_Debug:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Error);
                    break;
                case    ACTTraceLevel_Trace:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Detail);
                    break;
                case    ACTTraceLevel_Info:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Info);
                    break;
                case    ACTTraceLevel_Warn:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Warning);
                    break;
                case   ACTTraceLevel_Error:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Error);
                    break;
                case   ACTTraceLevel_Fatal:
                    ARIASDK_SET_LOG_LEVEL_(PAL::LogLevel::Error);
                    break;
                default:
                    break;
                }

				return result;
			}

            void LogManager::FlushAndTeardown()
            {
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    ARIASDK_LOG_DETAIL("FlushAndTeardown()");
                   
                    std::lock_guard<std::mutex> lock(*our_lockP);

                    if (nullptr != our_pLogManagerSingletonInstanceP)
                    {
                        LogManagerProvider::DestroyLogManager("OldInterface");
                        our_pLogManagerSingletonInstanceP = nullptr;
                    }
                }
                else 
                {
                    ARIASDK_LOG_DETAIL("FlushAndTeardown() has been already called.");
                }
            }

            void LogManager::UploadNow()
            {
                ARIASDK_LOG_DETAIL("UploadNow()");
                if (nullptr != our_LogController)
                {
                    our_LogController->UploadNow();
                }
            }

            /// <summary>
            /// Save all unsent records to offline storage.
            /// </summary>
            void LogManager::Flush()
            {
                ARIASDK_LOG_DETAIL("Flush()");
                if (nullptr != our_LogController)
                {
                    our_LogController->Flush();
                }
            }

            /// <summary>
            /// Pause telemetry event transmission
            /// </summary>
            void LogManager::PauseTransmission()
            {
                ARIASDK_LOG_DETAIL("PauseTransmission()");
                if (nullptr != our_LogController)
                {
                    our_LogController->PauseTransmission();
                }
            }

            /// <summary>
            /// Resume telemetry event transmission
            /// </summary>
            void LogManager::ResumeTransmission()
            {
                ARIASDK_LOG_DETAIL("ResumeTransmission()");
                if (nullptr != our_LogController)
                {
                    our_LogController->ResumeTransmission();
                }
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            void LogManager::SetTransmitProfile(TransmitProfile profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%d", profile);
                if (nullptr != our_LogController)
                {
                    our_LogController->SetTransmitProfile(profile);
                }
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            bool LogManager::SetTransmitProfile(const std::string& profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%s", profile.c_str());
                return TransmitProfiles::setProfile(profile);
            }

            /// <summary>
            /// Load transmit profiles from JSON config
            /// </summary>
            /// <param name="profiles_json">JSON config (see example above)</param>
            /// <returns>true on successful profiles load, false if config is invalid</returns>
            bool LogManager::LoadTransmitProfiles(const std::string& profiles_json)
            {
                ARIASDK_LOG_DETAIL("LoadTransmitProfiles");
                return TransmitProfiles::load(profiles_json);
            }

            /// <summary>
            /// Reset transmission profiles to default settings
            /// </summary>
            void LogManager::ResetTransmitProfiles()
            {
                ARIASDK_LOG_DETAIL("ResetTransmitProfiles");
                TransmitProfiles::reset();
            }

            const std::string LogManager::GetTransmitProfileName(TransmitProfile profile)
            {
				UNREFERENCED_PARAMETER(profile);
               return TransmitProfiles::getProfile();
            };

            /// <summary>
            /// Get global semantic context.
            /// </summary>
            /// <returns></returns>
			ISemanticContext* LogManager::GetSemanticContext()
            {
                ARIASDK_LOG_DETAIL("GetSemanticContext()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    ISemanticContext& semanticContext = our_pLogManagerSingletonInstanceP->GetSemanticContext();
                    return &semanticContext;
                }
                else
                {
                    return nullptr;
                }
            }

            /// <summary>
            /// Internal template to handle various field types
            /// </summary>
            /// <summary>
            /// <param name="name">name</param>
            /// <param name="value">value</param>
            /// <param name="piiKind">Pii kind</param>
            template <typename T>
            void LogManager::_SetContext(const string& name, T value, PiiKind piiKind)
            {
                ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
            }

            /// <summary>
            /// Set global context field - string
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, const std::string& value, PiiKind piiKind) 
			{
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            /// <summary>
            /// Set global context field - double
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, double value, PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            /// <summary>
            /// Set global context field - int64
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, int64_t value,            PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            /// <summary>
            /// Set global context field - boolean
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, bool value,               PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            /// <summary>
            /// Set global context field - date/time in .NET ticks
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, time_ticks_t value,       PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            /// <summary>
            /// Set global context field - GUID
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            void LogManager::SetContext(const std::string& name, GUID_t value,             PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
			}

            ILogger* LogManager::GetLogger()
            {
                ARIASDK_LOG_DETAIL("GetLogger()");
                return nullptr;
            }

            ILogger* LogManager::GetLogger(const string& source)
            {
                ARIASDK_LOG_DETAIL("GetLogger[1]: source=%s", source.c_str() );
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    return our_pLogManagerSingletonInstanceP->GetLogger(source);
                }
                else
                {
                    return nullptr;
                }
            }

            ILogger* LogManager::GetLogger(const std::string& tenantToken, const string& source)
            {
                ARIASDK_LOG_DETAIL("GetLogger[2]: tenantToken=%s, source=%s", tenantToken.c_str(), source.c_str() );
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    return our_pLogManagerSingletonInstanceP->GetLogger(tenantToken);
                }
                else
                {
                    return nullptr;
                }
            }

            __inline void LogManager::checkup() { };

            /// <summary>Add debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            void LogManager::AddEventListener(DebugEventType type, DebugEventListener &listener) 
            {
                if (nullptr != our_LogController)
                {
                    our_LogController->AddEventListener(type, listener);
                }
            }

            /// <summary>Remove debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            void LogManager::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
            {
                if (nullptr != our_LogController)
                {
                    our_LogController->RemoveEventListener(type, listener);
                }
            }

            ILogConfiguration& LogManager::GetLogConfiguration()
            {
                return *our_LogConfigurationP;;
            }
        }
    }
}
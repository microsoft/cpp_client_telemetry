#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "Enums.hpp"
#include "CommonLogManagerInternal.hpp"
#include "LogManagerImpl.hpp"
#include "TransmitProfiles.hpp"


#include <atomic>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

			std::mutex*          our_CommonLogManagerInternallockP = new std::mutex();
			ILogManagerInternal* our_pLogManagerSingletonInstanceP = nullptr;
            static volatile LONG our_CommonLogManagerInternalStarted = 0;
            HANDLE syncEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            LogSessionData*      our_LogSessionDataP = nullptr;
            bool                 our_IsRuningasHost = false;
            LogConfiguration*    our_LogConfiguration = new LogConfiguration();
            AuthTokensController* our_AuthTokenControllerP = new AuthTokensController();

            bool CommonLogManagerInternal::IsInitialized()
            {
                std::lock_guard<std::mutex> lock(*our_CommonLogManagerInternallockP);
                if (our_CommonLogManagerInternalStarted > 0)
                    return true;
                else
                    return false;
            }

            bool CommonLogManagerInternal::IsInitializedAsHost()
            {
                std::lock_guard<std::mutex> lock(*our_CommonLogManagerInternallockP);
                if (our_IsRuningasHost)
                    return true;
                else
                    return false;
            }

            ACTStatus CommonLogManagerInternal::Initialize( LogConfiguration* logConfigurationP, bool wantController)
			{
				ARIASDK_LOG_DETAIL("Initialize[1]:configuration=0x%X", logConfigurationP);
                ACTStatus status = ACTStatus::ACTStatus_OK;
				
                if (InterlockedIncrementAcquire(&our_CommonLogManagerInternalStarted) == 1)
                {
                    std::lock_guard<std::mutex> lock(*our_CommonLogManagerInternallockP);
                    if (nullptr == our_pLogManagerSingletonInstanceP)
                    {
                        if (logConfigurationP)
                        {
                            delete our_LogConfiguration;
                            our_LogConfiguration = logConfigurationP;
                        }
                      
                        our_pLogManagerSingletonInstanceP = ILogManagerInternal::Create(*our_LogConfiguration, nullptr);
                        ::SetEvent(syncEvent);

                        if (nullptr == our_LogSessionDataP)
                        {
                            ACTStatus error;
                            our_LogSessionDataP = new LogSessionData(our_LogConfiguration->GetProperty(CFG_STR_CACHE_FILE_PATH, error));
                        }
                        if (wantController & !our_IsRuningasHost)
                        {
                            our_IsRuningasHost = true;
                        }
                    }
                }
                else
                {
                    ::WaitForSingleObject(syncEvent, INFINITE);

                    if (wantController & !our_IsRuningasHost)
                    {// running with default config, means only gusts are running, Now host showed up, we ned to reset the system
                        our_IsRuningasHost = true;
                        if (logConfigurationP)
                        {
                            delete our_LogConfiguration;
                            our_LogConfiguration = logConfigurationP;
                        }
                        
                        std::lock_guard<std::mutex> lock(*our_CommonLogManagerInternallockP);
                        ::ResetEvent(syncEvent);
                        if (nullptr != our_pLogManagerSingletonInstanceP)
                        {
                            ILogManagerInternal* temp = ILogManagerInternal::Create(*our_LogConfiguration, nullptr);
                            our_pLogManagerSingletonInstanceP->FlushAndTeardown();
                            delete our_pLogManagerSingletonInstanceP;
                            our_pLogManagerSingletonInstanceP = temp;

                            ::SetEvent(syncEvent);
                        }
                    }
                    else
                    {
                        if (wantController)
                        {
                            status = ACTStatus::ACTStatus_AlreadyInitialized;
                        }
                    }
                }

                switch (our_LogConfiguration->GetMinimumTraceLevel())
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

				return status;
			}

            ACTStatus CommonLogManagerInternal::FlushAndTeardown()
            {
                ARIASDK_LOG_DETAIL("FlushAndTeardown()");
                if (InterlockedDecrementAcquire(&our_CommonLogManagerInternalStarted) == 0)
                {
                    std::lock_guard<std::mutex> lock(*our_CommonLogManagerInternallockP);
                    ::ResetEvent(syncEvent);
                    if (nullptr != our_pLogManagerSingletonInstanceP)
                    {                        
                        our_pLogManagerSingletonInstanceP->FlushAndTeardown();
                        delete our_pLogManagerSingletonInstanceP;
                        our_pLogManagerSingletonInstanceP = nullptr;
                    }
                    else
                    {
                        ACTStatus::ACTStatus_NoOp;
                    }
                }
                return ACTStatus::ACTStatus_OK;
            }

            ACTStatus CommonLogManagerInternal::UploadNow()
            {
                ARIASDK_LOG_DETAIL("UploadNow()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->UploadNow();
                }
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>
            /// Save all unsent records to offline storage.
            /// </summary>
            ACTStatus CommonLogManagerInternal::Flush()
            {
                ARIASDK_LOG_DETAIL("Flush()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->Flush();
                }
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>
            /// Pause telemetry event transmission
            /// </summary>
            ACTStatus CommonLogManagerInternal::PauseTransmission()
            {
                ARIASDK_LOG_DETAIL("PauseTransmission()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->PauseTransmission();
                }
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>
            /// Resume telemetry event transmission
            /// </summary>
            ACTStatus CommonLogManagerInternal::ResumeTransmission()
            {
                ARIASDK_LOG_DETAIL("ResumeTransmission()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->ResumeTransmission();
                }
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            ACTStatus CommonLogManagerInternal::SetTransmitProfile(TransmitProfile profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%d", profile);
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetTransmitProfile(profile);
                }
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            ACTStatus CommonLogManagerInternal::SetTransmitProfile(const std::string& profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%s", profile.c_str());
                if (TransmitProfiles::setProfile(profile))
                    return ACTStatus::ACTStatus_OK;
                else
                    return ACTStatus::ACTStatus_Fail;
            }

            /// <summary>
            /// Load transmit profiles from JSON config
            /// </summary>
            /// <param name="profiles_json">JSON config (see example above)</param>
            /// <returns>true on successful profiles load, false if config is invalid</returns>
            ACTStatus CommonLogManagerInternal::LoadTransmitProfiles(const std::string& profiles_json)
            {
                ARIASDK_LOG_DETAIL("LoadTransmitProfiles");
                if (TransmitProfiles::load(profiles_json))
                    return ACTStatus::ACTStatus_OK;
                else
                    return ACTStatus::ACTStatus_Fail;
            }

            /// <summary>
            /// Reset transmission profiles to default settings
            /// </summary>
            ACTStatus CommonLogManagerInternal::ResetTransmitProfiles()
            {
                ARIASDK_LOG_DETAIL("ResetTransmitProfiles");
                TransmitProfiles::reset();
                return ACTStatus::ACTStatus_OK;
            }

            const std::string& CommonLogManagerInternal::GetTransmitProfileName()
            {
               return TransmitProfiles::getProfile();
            };

            /// <summary>
            /// Get global semantic context.
            /// </summary>
            /// <returns></returns>
			ISemanticContext* CommonLogManagerInternal::GetSemanticContext()
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
            ACTStatus CommonLogManagerInternal::_SetContext(const string& name, T value, PiiKind piiKind)
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
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, const std::string& value, PiiKind piiKind) 
			{
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            /// <summary>
            /// Set global context field - double
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, double value, PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            /// <summary>
            /// Set global context field - int64
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, int64_t value,            PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            /// <summary>
            /// Set global context field - boolean
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, bool value,               PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            /// <summary>
            /// Set global context field - date/time in .NET ticks
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, time_ticks_t value,       PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            /// <summary>
            /// Set global context field - GUID
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            ACTStatus CommonLogManagerInternal::SetContext(const std::string& name, GUID_t value,             PiiKind piiKind) {
				ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, piiKind);
                }
                return ACTStatus::ACTStatus_OK;
			}

            ILogger* CommonLogManagerInternal::GetLogger(const std::string& tenantToken, ContextFieldsProvider* context)
            {
                ARIASDK_LOG_DETAIL("GetLogger[1]: tenantToken=%s", tenantToken.c_str());
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    return our_pLogManagerSingletonInstanceP->GetLogger(tenantToken, context);
                }
                else
                {
                    return nullptr;
                }
            }

            void CommonLogManagerInternal::AddIncomingEvent(IncomingEventContextPtr const& event)
            {
                ARIASDK_LOG_DETAIL("AddIncomingEvent");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    return our_pLogManagerSingletonInstanceP->addIncomingEvent(event);
                }
            }

            __inline ACTStatus CommonLogManagerInternal::checkup() { };

            /// <summary>Debug callback implementation</summary>
            static DebugEventSource debugEventSource;
            
            /// <summary>Add debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            ACTStatus CommonLogManagerInternal::AddEventListener(DebugEventType type, DebugEventListener &listener)
            {
                debugEventSource.AddEventListener(type, listener);
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>Remove debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            ACTStatus CommonLogManagerInternal::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
            {
                debugEventSource.RemoveEventListener(type, listener);
                return ACTStatus::ACTStatus_OK;
            }

            /// <summary>[internal] Dispatch debug event to debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            bool CommonLogManagerInternal::DispatchEvent(DebugEventType type) {
                return debugEventSource.DispatchEvent(type);
            }

            /// <summary>[internal] Dispatch debug event to debug event listener.</summary>
            /// <param name="evt"></param>
            /// <returns></returns>
            bool CommonLogManagerInternal::DispatchEvent(DebugEvent &evt) {
                return debugEventSource.DispatchEvent(evt);
            }

            LogSessionData* CommonLogManagerInternal::GetLogSessionData()
            {
                return our_LogSessionDataP;
            }

            /// <summary>
            /// Get AuthTokens controller
            /// </summary>
            AuthTokensController* CommonLogManagerInternal::GetAuthTokensController()
            {
                return our_AuthTokenControllerP;
            }
        }
    }
}
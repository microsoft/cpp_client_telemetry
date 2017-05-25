#define LOG_MODULE DBG_API

#include "LogManager.hpp"
#include "LogManagerImpl.hpp"

#include "tpm/TransmitProfiles.hpp"


#include <atomic>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

			std::mutex*          our_lockP = new std::mutex();
			ILogManager*         our_pLogManagerSingletonInstanceP = nullptr;
			LogSessionData*      our_LogSessionDataP = nullptr;

            static volatile std::atomic_bool isInited = ATOMIC_VAR_INIT(false);

			ILogger* LogManager::Initialize(const std::string& tenantToken)
			{
				return LogManager::Initialize(tenantToken, LogConfiguration());
			}

			ILogger* LogManager::Initialize(const std::string& tenantToken, const LogConfiguration& configuration)
			{
				ARIASDK_LOG_DETAIL("Initialize[2]: tenantToken=%s, configuration=0x%X", tenantToken.c_str(), &configuration);
				std::lock_guard<std::mutex> lock(*our_lockP);

				if (nullptr == our_pLogManagerSingletonInstanceP)
				{
					our_pLogManagerSingletonInstanceP = ILogManager::Create(configuration);
				}

				if (nullptr == our_LogSessionDataP)
				{
					our_LogSessionDataP = new LogSessionData(configuration.cacheFilePath);
				}

				ILogger *result = our_pLogManagerSingletonInstanceP->GetLogger(tenantToken);
				//isInited = result->IsInitialized();
				return result;
			}

            void LogManager::FlushAndTeardown()
            {
                if (isInited.exchange(false)) {
                    ARIASDK_LOG_DETAIL("FlushAndTeardown()");
                   
                    if (nullptr != our_pLogManagerSingletonInstanceP)
                    {
                        our_pLogManagerSingletonInstanceP->FlushAndTeardown();
                    }
                }
                else {
                    ARIASDK_LOG_DETAIL("FlushAndTeardown() has been already called.");
                }
            }

            void LogManager::UploadNow()
            {
                ARIASDK_LOG_DETAIL("UploadNow()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->UploadNow();
                }
            }

            /// <summary>
            /// Save all unsent records to offline storage.
            /// </summary>
            void LogManager::Flush()
            {
                ARIASDK_LOG_DETAIL("Flush()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->Flush();
                }
            }

            /// <summary>
            /// Pause telemetry event transmission
            /// </summary>
            void LogManager::PauseTransmission()
            {
                ARIASDK_LOG_DETAIL("PauseTransmission()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->PauseTransmission();
                }
            }

            /// <summary>
            /// Resume telemetry event transmission
            /// </summary>
            void LogManager::ResumeTransmission()
            {
                ARIASDK_LOG_DETAIL("ResumeTransmission()");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->ResumeTransmission();
                }
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            void LogManager::SetTransmitProfile(TransmitProfile profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%d", profile);
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetTransmitProfile(profile);
                }
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            bool LogManager::SetTransmitProfile(const std::string& profile)
            {
                ARIASDK_LOG_DETAIL("SetTransmitProfile: profile=%s", profile.c_str());
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    return our_pLogManagerSingletonInstanceP->SetTransmitProfile(profile);
                }
                else
                {
                    return false;
                }
            }

            /// <summary>
            /// Load transmit profiles from JSON config
            /// </summary>
            /// <param name="profiles_json">JSON config (see example above)</param>
            /// <returns>true on successful profiles load, false if config is invalid</returns>
            bool LogManager::LoadTransmitProfiles(std::string profiles_json)
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

            const std::string& LogManager::GetTransmitProfileName(TransmitProfile profile)
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
                    return our_pLogManagerSingletonInstanceP->GetLogger(tenantToken, source);
                }
                else
                {
                    return nullptr;
                }
            }

            __inline void LogManager::checkup() { };

            /// <summary>Debug callback implementation</summary>
            static DebugEventSource debugEventSource;
            
            /// <summary>Add debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            void LogManager::AddEventListener(DebugEventType type, DebugEventListener &listener) {
                debugEventSource.AddEventListener(type, listener);
            }

            /// <summary>Remove debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            void LogManager::RemoveEventListener(DebugEventType type, DebugEventListener &listener) {
                debugEventSource.RemoveEventListener(type, listener);
            }

            /// <summary>[internal] Dispatch debug event to debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            bool LogManager::DispatchEvent(DebugEventType type) {
                return debugEventSource.DispatchEvent(type);
            }

            /// <summary>[internal] Dispatch debug event to debug event listener.</summary>
            /// <param name="evt"></param>
            /// <returns></returns>
            bool LogManager::DispatchEvent(DebugEvent &evt) {
                return debugEventSource.DispatchEvent(evt);
            }

			LogSessionData* LogManager::GetLogSessionData() 
			{
				return our_LogSessionDataP;;
			}
        }
    }
}
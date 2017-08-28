#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "LogManager.hpp"
#include "LogSessionData.hpp"
#include "LogManagerImpl.hpp"

#include "TransmitProfiles.hpp"


#include <atomic>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

			std::mutex*          our_lockP = new std::mutex();
			ILogManager*         our_pLogManagerSingletonInstanceP = nullptr;
			LogSessionData*      our_LogSessionDataP = nullptr;

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
					our_LogSessionDataP = new LogSessionData(configuration.GetProperty(CFG_STR_CACHE_FILE_PATH));
				}

				ILogger *result = our_pLogManagerSingletonInstanceP->GetLogger(tenantToken);

                switch (configuration.minimumTraceLevel)
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
                        our_pLogManagerSingletonInstanceP->FlushAndTeardown();
                        delete our_pLogManagerSingletonInstanceP;
                        our_pLogManagerSingletonInstanceP = nullptr;
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
				return TransmitProfiles::setProfile(profile);
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
            /// Adds or overrides a property of the custom context for the telemetry logging system.
            /// Context information set here applies to events generated by all ILogger instances
            /// unless it is overwritten on a particular ILogger instance.
            /// </summary>
            /// <param name="name">Name of the context property</param>
            /// <param name="value">Value of the context property</param>
            /// <param name='ccKind'>Customer content kind</param>
            void LogManager::SetContext(const std::string& name, const std::string& value, CustomerContentKind ccKind)
            {
                ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, ccKind);
                }
            }

            /// <summary>
            /// Adds or overrides a property of the custom context for the telemetry logging system.
            /// Context information set here applies to events generated by all ILogger instances
            /// unless it is overwritten on a particular ILogger instance.
            /// </summary>
            /// <param name="name">Name of the context property</param>
            /// <param name="value">Value of the context property</param>
            /// <param name='ccKind'>Customer content kind</param>
            void LogManager::SetContext(const std::string& name, const char *value, CustomerContentKind ccKind)
            {
                ARIASDK_LOG_DETAIL("SetContext");
                if (nullptr != our_pLogManagerSingletonInstanceP)
                {
                    our_pLogManagerSingletonInstanceP->SetContext(name, value, ccKind);
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
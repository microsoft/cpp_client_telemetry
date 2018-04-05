#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "LogManagerSimple.hpp"
#include "LogManagerProvider.hpp"
#include "HostGuestLogManager.hpp"
#include "LogConfiguration.hpp"
#include "TransmitProfiles.hpp"


#include <atomic>

using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Events  {

			std::mutex*          our_lockSimpleP = new std::mutex();
			ILogManager*         our_pLogManagerSimpleSingletonInstanceP = nullptr;
            LogConfiguration*    our_LogSimpleConfigurationP = new LogConfiguration();
            ILogController*      our_LogSimpleController = nullptr;
            IAuthTokensController* our_AuthTokenSimpleController = nullptr;

            IAuthTokensController* LogManagerSimple::GetAuthTokenController()
            {
                return our_AuthTokenSimpleController;
            }

            EVTStatus LogManagerSimple::Start()
			{
				std::lock_guard<std::mutex> lock(*our_lockSimpleP);

				if (nullptr == our_pLogManagerSimpleSingletonInstanceP)
				{
                    EVTStatus error;
                  	our_pLogManagerSimpleSingletonInstanceP = LogManagerProvider::CreateLogManager("SimpleInterface",true,*our_LogSimpleConfigurationP, error);
                    our_LogSimpleController = our_pLogManagerSimpleSingletonInstanceP->GetLogController();
                    our_AuthTokenSimpleController = our_pLogManagerSimpleSingletonInstanceP->GetAuthTokensController();
                    return EVTStatus::EVTStatus_OK;
				}

				return EVTStatus::EVTStatus_AlreadyInitialized;
			}

            EVTStatus LogManagerSimple::Teardown()
            {
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    LOG_TRACE("FlushAndTeardown()");
                   
                    std::lock_guard<std::mutex> lock(*our_lockSimpleP);

                    if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                    {
                        LogManagerProvider::DestroyLogManager("SimpleInterface");
                        our_pLogManagerSimpleSingletonInstanceP = nullptr;
                    }
                    return EVTStatus::EVTStatus_OK;
                }
                else 
                {
                    LOG_TRACE("FlushAndTeardown() has been already called.");
                    return EVTStatus::EVTStatus_NoOp;
                }
            }

            EVTStatus LogManagerSimple::UploadNow()
            {
                LOG_TRACE("UploadNow()");
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->UploadNow();
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Save all unsent records to offline storage.
            /// </summary>
            EVTStatus LogManagerSimple::Flush()
            {
                LOG_TRACE("Flush()");
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->Flush();
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Pause telemetry event transmission
            /// </summary>
            EVTStatus LogManagerSimple::PauseTransmission()
            {
                LOG_TRACE("PauseTransmission()");
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->PauseTransmission();
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Resume telemetry event transmission
            /// </summary>
            EVTStatus LogManagerSimple::ResumeTransmission()
            {
                LOG_TRACE("ResumeTransmission()");
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->ResumeTransmission();
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            EVTStatus LogManagerSimple::SetTransmitProfile(TransmitProfile profile)
            {
                LOG_TRACE("SetTransmitProfile: profile=%d", profile);
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->SetTransmitProfile(profile);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Select one of several predefined transmission profiles.
            /// </summary>
            /// <param name="profile"></param>
            bool LogManagerSimple::SetTransmitProfile(const std::string& profile)
            {
                LOG_TRACE("SetTransmitProfile: profile=%s", profile.c_str());
                return TransmitProfiles::setProfile(profile);
            }

            /// <summary>
            /// Load transmit profiles from JSON config
            /// </summary>
            /// <param name="profiles_json">JSON config (see example above)</param>
            /// <returns>true on successful profiles load, false if config is invalid</returns>
            bool LogManagerSimple::LoadTransmitProfiles(const std::string& profiles_json)
            {
                LOG_TRACE("LoadTransmitProfiles");
                return TransmitProfiles::load(profiles_json);
            }

            /// <summary>
            /// Reset transmission profiles to default settings
            /// </summary>
            EVTStatus LogManagerSimple::ResetTransmitProfiles()
            {
                LOG_TRACE("ResetTransmitProfiles");
                TransmitProfiles::reset();
                return EVTStatus::EVTStatus_OK;
            }

            const std::string LogManagerSimple::GetTransmitProfileName(TransmitProfile profile)
            {
				UNREFERENCED_PARAMETER(profile);
               return TransmitProfiles::getProfile();
            };

            /// <summary>
            /// Get global semantic context.
            /// </summary>
            /// <returns></returns>
			ISemanticContext* LogManagerSimple::GetSemanticContext()
            {
                LOG_TRACE("GetSemanticContext()");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    ISemanticContext& semanticContext = our_pLogManagerSimpleSingletonInstanceP->GetSemanticContext();
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
            EVTStatus LogManagerSimple::_SetContext(const string& name, T value, PiiKind piiKind)
            {
                LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>
            /// Set global context field - string
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, const std::string& value, PiiKind piiKind) 
			{
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            /// <summary>
            /// Set global context field - double
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, double value, PiiKind piiKind) {
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            /// <summary>
            /// Set global context field - int64
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, int64_t value,            PiiKind piiKind) {
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            /// <summary>
            /// Set global context field - boolean
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, bool value,               PiiKind piiKind) {
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            /// <summary>
            /// Set global context field - date/time in .NET ticks
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, time_ticks_t value,       PiiKind piiKind) {
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            /// <summary>
            /// Set global context field - GUID
            /// </summary>
            /// <param name="name"></param>
            /// <param name="value"></param>
            /// <param name="piiKind"></param>
            EVTStatus LogManagerSimple::SetContext(const std::string& name, GUID_t value,             PiiKind piiKind) {
				LOG_TRACE("SetContext");
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    our_pLogManagerSimpleSingletonInstanceP->SetContext(name, value, piiKind);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
			}

            ILogger* LogManagerSimple::GetLogger()
            {
                LOG_TRACE("GetLogger()");
                return nullptr;
            }

            ILogger* LogManagerSimple::GetLogger(const string& source)
            {
                LOG_TRACE("GetLogger[1]: source=%s", source.c_str() );
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    return our_pLogManagerSimpleSingletonInstanceP->GetLogger(source);
                }
                else
                {
                    return nullptr;
                }
            }

            ILogger* LogManagerSimple::GetLogger(const std::string& tenantToken, const string& source)
            {
                LOG_TRACE("GetLogger[2]: tenantToken=%s, source=%s", tenantToken.c_str(), source.c_str() );
                if (nullptr != our_pLogManagerSimpleSingletonInstanceP)
                {
                    return our_pLogManagerSimpleSingletonInstanceP->GetLogger(tenantToken);
                }
                else
                {
                    return nullptr;
                }
            }

            __inline EVTStatus LogManagerSimple::checkup() { return EVTStatus::EVTStatus_OK; };

            /// <summary>Add debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            EVTStatus LogManagerSimple::AddEventListener(DebugEventType type, DebugEventListener &listener) 
            {
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->AddEventListener(type, listener);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            /// <summary>Remove debug event listener.</summary>
            /// <param name="type"></param>
            /// <param name="listener"></param>
            EVTStatus LogManagerSimple::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
            {
                if (nullptr != our_LogSimpleController)
                {
                    our_LogSimpleController->RemoveEventListener(type, listener);
                    return EVTStatus::EVTStatus_OK;
                }
                return EVTStatus::EVTStatus_NoOp;
            }

            ILogConfiguration& LogManagerSimple::GetLogConfiguration()
            {
                return *our_LogSimpleConfigurationP;;
            }
        }
    }
}
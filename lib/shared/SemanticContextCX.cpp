#include "pch.h"
#include "SemanticContextCX.hpp"

namespace MAT = Microsoft::Applications::Telemetry;

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                void SemanticContextImpl::SetUserId(String ^ userId, PiiKind piiKind)
                {
                    m_semanticContextCore->SetUserId(FromPlatformString(userId), (MAT::PiiKind)piiKind);
                }
                SemanticContextImpl::SemanticContextImpl(MAT::ISemanticContext* semanticContextCore) :
                    m_semanticContextCore(semanticContextCore)
                {
                }

                void SemanticContextImpl::AppId::set(String^ appId)
                {
                    m_semanticContextCore->SetAppId(FromPlatformString(appId));
                }

                void SemanticContextImpl::AppVersion::set(String^ appVersion)
                {
                    m_semanticContextCore->SetAppVersion(FromPlatformString(appVersion));
                }

                void SemanticContextImpl::AppExperimentETag::set(String^ appExperimentETag)
                {
                    m_semanticContextCore->SetAppExperimentETag(FromPlatformString(appExperimentETag));
                }

                void SemanticContextImpl::AppExperimentIds::set(String^ appExperimentIds)
                {
                    m_semanticContextCore->SetAppExperimentIds(FromPlatformString(appExperimentIds));
                }

                void SemanticContextImpl::SetEventExperimentIds(String^ eventName, String^ experimentIds)
                {
                    m_semanticContextCore->SetEventExperimentIds(FromPlatformString(eventName), FromPlatformString(experimentIds));
                }

                void SemanticContextImpl::UserId::set(String^ userId)
                {
                    m_semanticContextCore->SetUserId(FromPlatformString(userId));
                }

                void SemanticContextImpl::UserMsaId::set(String^ userMsaId)
                {
                    m_semanticContextCore->SetUserMsaId(FromPlatformString(userMsaId));
                }

                void SemanticContextImpl::UserANID::set(String^ userANID)
                {
                    m_semanticContextCore->SetUserANID(FromPlatformString(userANID));
                }

                void SemanticContextImpl::UserAdvertisingId::set(String^ userAdvertisingId)
                {
                    m_semanticContextCore->SetUserAdvertisingId(FromPlatformString(userAdvertisingId));
                }

                void SemanticContextImpl::UserLanguage::set(String^ userLanguage)
                {
                    m_semanticContextCore->SetUserLanguage(FromPlatformString(userLanguage));
                }

                void SemanticContextImpl::UserTimeZone::set(String^ userTimeZone)
                {
                    m_semanticContextCore->SetUserTimeZone(FromPlatformString(userTimeZone));
                }
            }
        }
    }
}
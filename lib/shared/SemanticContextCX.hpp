//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"
#include "ISemanticContext.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                public interface class ISemanticContext
                {
                    property String^ AppEnv
                    {
                        virtual void set(String^ appEnv) = 0;
                    }

                    property String^ AppId
                    {
                        virtual void set(String^ appId) = 0;
                    }

                    property String^ AppName
                    {
                        virtual void set(String^ appName) = 0;
                    }

                    property String^ AppVersion
                    {
                        virtual void set(String^ appVersion) = 0;
                    }

                    property String^ DeviceId
                    {
                        virtual void set(String^ deviceId) = 0;
                    }

                    property String^ DeviceMake
                    {
                        virtual void set(String^ deviceMake) = 0;
                    }

                    property String^ DeviceModel
                    {
                        virtual void set(String^ deviceModel) = 0;
                    }

                    property String^ NetworkProvider
                    {
                        virtual void set(String^ networkProvider) = 0;
                    }

                    property String^ OsName
                    {
                        virtual void set(String^ osName) = 0;
                    }

                    property String^ OsVersion
                    {
                        virtual void set(String^ osVersion) = 0;
                    }

                    property String^ OsBuild
                    {
                        virtual void set(String^ osBuild) = 0;
                    }

                    property String^ UserId
                    {
                        virtual void set(String^ userId) = 0;
                    }

                    property String^ UserMsaId
                    {
                        virtual void set(String^ userMsaId) = 0;
                    }

                    property String^ UserANID
                    {
                        virtual void set(String^ userANID) = 0;
                    }

                    property String^ UserAdvertisingId
                    {
                        virtual void set(String^ userAdvertisingId) = 0;
                    }

                    property String^ UserLanguage
                    {
                        virtual void set(String^ UserLanguage) = 0;
                    }
               
                    property String^ UserTimeZone
                    {
                        virtual void set(String^ userTimeZone) = 0;
                    }

                    virtual void SetUserId(String^ userId, PiiKind piiKind) = 0;
                };

                /// @cond INTERNAL_DOCS
                /// Excluded from public docs
                ref class SemanticContextImpl sealed : ISemanticContext
                {
                public:

                    // Inherited via ISemanticContext
                    property String ^ AppEnv {
                         virtual void set(String ^ appEnv);
                    }

                    // Inherited via ISemanticContext
                    property String^ AppId
                    {
                        virtual void set(String^ appId);
                    }

                    // Inherited via ISemanticContext
                    property String^ AppName
                    {
                        virtual void set(String^ appName);
                    }

                    property String^ AppVersion
                    {
                        virtual void set(String^ appVersion);
                    }

                    property String^ DeviceId
                    {
                        virtual void set(String^ deviceId);
                    }

                    property String^ DeviceMake
                    {
                        virtual void set(String^ deviceMake);
                    }

                    property String^ DeviceModel
                    {
                        virtual void set(String^ deviceModel);
                    }

                    property String^ NetworkProvider
                    {
                        virtual void set(String^ networkProvider);
                    }

                    property String^ OsName
                    {
                        virtual void set(String^ osName);
                    }

                    property String^ OsVersion
                    {
                        virtual void set(String^ osVersion);
                    }

                    property String^ OsBuild
                    {
                        virtual void set(String^ osBuild);
                    }

                    property String^ UserId
                    {
                        virtual void set(String^ userId);
                    }

                    property String^ UserMsaId
                    {
                        virtual void set(String^ userMsaId);
                    }

                    property String^ UserANID
                    {
                        virtual void set(String^ userANID);
                    }

                    property String^ UserAdvertisingId
                    {
                        virtual void set(String^ userAdvertisingId);
                    }

                    property String^ UserLanguage
                    {
                        virtual void set(String^ userLanguage);
                    }

                    property String^ UserTimeZone
                    {
                        virtual void set(String^ userTimeZone);
                    }

                    virtual void SetUserId(String^ userId, PiiKind piiKind);

                    property String^ AppExperimentETag
                    {
                        virtual void set(String^ appExperimentETag);
                    }
                    
                    property String^ AppExperimentIds
                    {
                        virtual void set(String^ appExperimentIds);
                    }
                                        
                    virtual void SetEventExperimentIds(String^ eventName, String^ experimentIds);					

                internal:
                    SemanticContextImpl(MAT::ISemanticContext* semanticContextCore);

                private:
                    MAT::ISemanticContext* m_semanticContextCore;

                };
                /// @endcond
            }
        }
    }
}


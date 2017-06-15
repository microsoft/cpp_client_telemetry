// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <ISemanticContext.hpp>

namespace testing {


class MockISemanticContext : public ARIASDK_NS::ISemanticContext {
  public:
    MockISemanticContext();
    virtual ~MockISemanticContext();

    MOCK_METHOD1(SetAppId,             void(std::string const & appId));
    MOCK_METHOD1(SetAppVersion,        void(std::string const & appVersion));
    MOCK_METHOD1(SetAppLanguage,       void(std::string const & appLanguage));
    MOCK_METHOD1(SetAppExperimentIds,  void(std::string const & appExperimentIds));
    MOCK_METHOD1(SetDeviceId,          void(std::string const & deviceId));
    MOCK_METHOD1(SetDeviceMake,        void(std::string const & deviceMake));
    MOCK_METHOD1(SetDeviceModel,       void(std::string const & deviceModel));
    MOCK_METHOD1(SetNetworkCost,       void(ARIASDK_NS::NetworkCost networkCost));
    MOCK_METHOD1(SetNetworkProvider,   void(std::string const & networkProvider));
    MOCK_METHOD1(SetNetworkType,       void(ARIASDK_NS::NetworkType networkType));
    MOCK_METHOD1(SetOsName,            void(std::string const & osName));
    MOCK_METHOD1(SetOsVersion,         void(std::string const & osVersion));
    MOCK_METHOD1(SetOsBuild,           void(std::string const & osBuild));
    MOCK_METHOD2(SetUserId,            void(std::string const & userId, ARIASDK_NS::PiiKind piiKind));
    MOCK_METHOD1(SetUserMsaId,         void(std::string const & userMsaId));
    MOCK_METHOD1(SetUserANID,          void(std::string const & userANID));
    MOCK_METHOD1(SetUserAdvertisingId, void(std::string const & userAdvertingId));
    MOCK_METHOD1(SetUserLanguage,      void(std::string const & locale));
    MOCK_METHOD1(SetUserTimeZone,      void(std::string const & timeZone));
	MOCK_METHOD1(SetAppExperimentETag, void(std::string const& appExperimentETag));
	MOCK_METHOD1(SetAppExperimentImpressionId, void(std::string const& appExperimentImpressionId));
	MOCK_METHOD2(SetEventExperimentIds, void(std::string const& eventName, std::string const& experimentIds));
};


} // namespace testing

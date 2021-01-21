//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <api/ContextFieldsProvider.hpp>

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

class MockISemanticContext : public MAT::ContextFieldsProvider {
  public:
    MockISemanticContext();
    virtual ~MockISemanticContext();

    MOCK_METHOD1(SetAppId,             void(std::string const & appId));
    MOCK_METHOD1(SetAppName,           void(std::string const & appName));
    MOCK_METHOD1(SetAppVersion,        void(std::string const & appVersion));
    MOCK_METHOD1(SetAppLanguage,       void(std::string const & appLanguage));
    MOCK_METHOD1(SetAppExperimentIds,  void(std::string const & appExperimentIds));
    MOCK_METHOD2(SetUserId,            void(std::string const & userId, MAT::PiiKind piiKind));
    MOCK_METHOD1(SetUserMsaId,         void(std::string const & userMsaId));
    MOCK_METHOD1(SetUserANID,          void(std::string const & userANID));
    MOCK_METHOD1(SetUserAdvertisingId, void(std::string const & userAdvertingId));
    MOCK_METHOD1(SetCommercialId,      void(std::string const & commercialId));
    MOCK_METHOD1(SetUserLanguage,      void(std::string const & locale));
    MOCK_METHOD1(SetUserTimeZone,      void(std::string const & timeZone));
    MOCK_METHOD1(SetAppExperimentETag, void(std::string const& appExperimentETag));
    MOCK_METHOD1(SetAppExperimentImpressionId, void(std::string const& appExperimentImpressionId));
    MOCK_METHOD2(SetEventExperimentIds, void(std::string const& eventName, std::string const& experimentIds));
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing


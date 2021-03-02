//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <ecsClientInterface.hpp>

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

#ifdef _MSC_VER
// Avoid noise caused by ECS using const modifier on value-type arguments like int, bool and double.
// The modifiers are lost inside Google Mock, the resulting signature differs and compiler complains.
#pragma warning(push)
#pragma warning(disable:4373) // C4373: previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers
#endif

class MockIEcsConfig : public ecsclient::IEcsConfig,
                       public virtual auf::Object
{
  public:
    MockIEcsConfig();
    virtual ~MockIEcsConfig();

    MOCK_CONST_METHOD3(GetSetting, std::string(std::string const & t, std::string const &, std::string const &));
    MOCK_CONST_METHOD3(GetSettingAsString, std::string(std::string const &, std::string const &, std::string const &));
    MOCK_CONST_METHOD3(GetSettingAsInt, int(std::string const &, std::string const &, int const defaultValue));
    MOCK_CONST_METHOD3(GetSettingAsBool, bool(std::string const &, std::string const &, bool const defaultValue));
    MOCK_CONST_METHOD3(GetSettingAsReal, double(std::string const & t, std::string const &, double const defaultValue));
    MOCK_CONST_METHOD2(GetSettingAsList, std::vector<std::string>(std::string const &, std::string const &));
    MOCK_CONST_METHOD2(GetSettingAsListOfInts, std::vector<int>(std::string const &, std::string const &));
    MOCK_CONST_METHOD2(GetSettingAsListOfReal, std::vector<double>(std::string const &, std::string const &));
    MOCK_CONST_METHOD2(GetKeys, std::vector<std::string>(std::string const &, std::string const &));
    MOCK_CONST_METHOD0(GetAgents, std::vector<std::string>());
    MOCK_CONST_METHOD0(GetETag, std::string());
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif


class MockIEcsClient : public ecsclient::IEcsClient {
  public:
    MockIEcsClient();
    virtual ~MockIEcsClient();

    MOCK_METHOD2(Initialize, ecsclient::ECS_ERROR(ecsclient::EcsClientConfig const &, ecsclient::EcsClientDependencies const &));
    MOCK_METHOD0(Start, ecsclient::ECS_ERROR());
    MOCK_METHOD1(Start, ecsclient::ECS_ERROR(char const*));
    MOCK_METHOD0(Stop, ecsclient::ECS_ERROR());
    MOCK_METHOD1(AddListener, ecsclient::ECS_ERROR(ecsclient::IEcsCallback*));
    MOCK_METHOD1(RemoveListener, ecsclient::ECS_ERROR(ecsclient::IEcsCallback*));
    MOCK_CONST_METHOD0(GetCurrentConfig, ecsclient::IEcsConfigPtr());
    MOCK_METHOD1(SetSkypeToken, void(std::string const &));
    MOCK_METHOD2(SetSkypeToken, void(std::string const &, std::string const &));
    MOCK_METHOD1(SetUserConfigFetchDelay, void(int delayInMs));
    MOCK_METHOD3(AddQueryParameter, void(char const*, char const*, char const*));
    MOCK_METHOD2(RemoveQueryParameter, void(char const*, char const*));
    MOCK_METHOD1(RemoveQueryParameterByNamespace, void(char const*));
    MOCK_METHOD0(SuspendSendingRequests, void());
    MOCK_METHOD0(ResumeSendingRequests, void());
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing


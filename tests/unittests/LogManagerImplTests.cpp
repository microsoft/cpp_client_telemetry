// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/LogManagerImpl.hpp"

using namespace testing;
using namespace MAT;

class TestLogManagerImpl : public LogManagerImpl
{
public:
	TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient)
		: LogManagerImpl(configuration, httpClient) { }
	using LogManagerImpl::m_httpClient;
	using LogManagerImpl::m_ownHttpClient;
};

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_ConstructsOwnHttpClient)
{
	ILogConfiguration configuration;
	TestLogManagerImpl logManager { configuration, nullptr };
	ASSERT_NE(logManager.m_ownHttpClient, nullptr);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_HttpClientAndOwnHttpClientAreSame)
{
	ILogConfiguration configuration;
	TestLogManagerImpl logManager { configuration, nullptr };
	ASSERT_EQ(logManager.m_ownHttpClient.get(), logManager.m_httpClient);
}
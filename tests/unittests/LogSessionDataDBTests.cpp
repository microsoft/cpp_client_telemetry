//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "offline/LogSessionDataProvider.hpp"
#include "LogSessionData.hpp"
#include "ITaskDispatcher.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "NullObjects.hpp"

#include<memory>
#include<functional>
#include<iostream>

namespace MAE = ::Microsoft::Applications::Events;
using namespace testing;

#define EXPECT_IN_RANGE(VAL, MIN, MAX) \
        EXPECT_GE((VAL), (MIN));       \
        EXPECT_LE((VAL), (MAX))

class TestTaskDispatcher_db : public ITaskDispatcher {
public:
    virtual void Join() {}
    virtual void Queue(Task *task) 
    {
        UNREFERENCED_PARAMETER(task);
    }
    virtual bool Cancel(Task *task, uint64_t waitTime = 0 )
    {
        UNREFERENCED_PARAMETER(task);
        UNREFERENCED_PARAMETER(waitTime);
        return true;
    }
    ~TestTaskDispatcher_db() {}
};

class LogSessionDataDBTests : public ::testing::Test
{
public:
    NullLogManager logManager;
    std::shared_ptr<OfflineStorageHandler> offlineStorage;
    LogSessionData *logSessionData;
    TestTaskDispatcher_db taskDispatcher;
    StrictMock<MockIOfflineStorageObserver> observerMock;
    StrictMock<MockIRuntimeConfig>  configMock;
    LogSessionDataProvider *logSessionDataProvider;
    std::ostringstream name;
    unsigned long long now = PAL::getUtcSystemTimeMs(); 

    virtual void SetUp() override
    {
    	EXPECT_CALL(configMock, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(
        		Return(32 * 4096));
    	EXPECT_CALL(configMock, GetMaximumRetryCount()).WillRepeatedly(
            	Return(5));
        EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"))
                .RetiresOnSaturation();
        name << MAE::GetTempDirectory() << "LogSessionDbSQLite.db";
        configMock[CFG_STR_CACHE_FILE_PATH] = name.str();
        std::remove(name.str().c_str());
        offlineStorage.reset(new OfflineStorageHandler(logManager, configMock, taskDispatcher)); 
		logSessionDataProvider = new  LogSessionDataProvider(offlineStorage.get());
        logSessionDataProvider->CreateLogSessionData();
        offlineStorage->Initialize(observerMock);
        logSessionDataProvider->CreateLogSessionData();
    }

    virtual void TearDown() override
    {
        std::remove(name.str().c_str());
        offlineStorage->Shutdown();
        offlineStorage.reset();
    }
};

TEST_F(LogSessionDataDBTests, subTest) {

#ifndef USE_ROOM
    logSessionData =  logSessionDataProvider->GetLogSessionData();
    auto sessionFirstTime= logSessionData->getSessionFirstTime();
    EXPECT_IN_RANGE(sessionFirstTime, now , now + 1000);
    auto sdkUid = logSessionData->getSessionSDKUid();
    EXPECT_TRUE(sdkUid.size());

    // fetch next time, we should get same values
    logSessionData =  logSessionDataProvider->GetLogSessionData();
    auto sessionFirstTime_again = logSessionData->getSessionFirstTime();
    auto sdkUid_again = logSessionData->getSessionSDKUid();
    EXPECT_EQ(sessionFirstTime, sessionFirstTime_again);
    EXPECT_EQ(sdkUid, sdkUid_again);
#else
    ASSERT_EQ(1, 1);
#endif
}


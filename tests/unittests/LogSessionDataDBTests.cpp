#include "common/Common.hpp"
#include "offline/LogSessionDataDB.hpp"
#include "ITaskDispatcher.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "NullObjects.hpp"

#include<memory>
#include<functional>
#include<iostream>

#define EXPECT_IN_RANGE(VAL, MIN, MAX) \
    EXPECT_GE((VAL), (MIN));           \
    EXPECT_LE((VAL), (MAX))

namespace MAE = ::Microsoft::Applications::Events;
using namespace testing;

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
    LogSessionDataDB *logSession;
    TestTaskDispatcher_db taskDispatcher;
    StrictMock<MockIOfflineStorageObserver> observerMock;
    StrictMock<MockIRuntimeConfig>  configMock;
    std::ostringstream name;

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
        logSession = new LogSessionDataDB(offlineStorage);
        offlineStorage->Initialize(observerMock);
    }   

    virtual void TearDown() override
    {
        std::remove(name.str().c_str());
        offlineStorage->Shutdown();
        offlineStorage.reset();
        delete logSession;
    }
};

TEST_F(LogSessionDataDBTests, subTest) {
#ifndef USE_ROOM
    unsigned long long now = PAL::getUtcSystemTimeMs();
    auto sessionFirstTime= logSession->getSessionFirstTime();
    EXPECT_IN_RANGE(sessionFirstTime, now , now+10);
    auto sdkUid = logSession->getSessionSDKUid();
    EXPECT_TRUE(sdkUid.size());

    // fetch next time, we should get same values
    auto sessionFirstTime_again = logSession->getSessionFirstTime();
    auto sdkUid_again = logSession->getSessionSDKUid();
    EXPECT_EQ(sessionFirstTime, sessionFirstTime_again);
    EXPECT_EQ(sdkUid, sdkUid_again);
#else
    ASSERT_EQ(1, 1);
#endif
}

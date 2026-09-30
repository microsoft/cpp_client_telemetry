//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#include "common/Common.hpp"
#include "api/DataViewerCollection.hpp"
#include "CheckForExceptionOrAbort.hpp"

#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

using namespace testing;
using namespace MAT;

class MockIDataViewer : public IDataViewer
{
   public:

    MockIDataViewer(const char* name, bool isTransmissionEnabled) :
        m_name(name), m_isTransmissionEnabled(isTransmissionEnabled) {}

    void ReceiveData(const std::vector<uint8_t>& packetData) noexcept override
    {
        localPacketData = packetData;
    }

    const char* GetName() const noexcept override
    {
        return m_name;
    }

    bool IsTransmissionEnabled() const noexcept override
    {
        return m_isTransmissionEnabled;
    }

    const std::string& GetCurrentEndpoint() const noexcept override
    {
        return m_testEndpoint;
    }

    mutable std::vector<uint8_t> localPacketData;
    const char* m_name;
    bool m_isTransmissionEnabled;
    const std::string m_testEndpoint{"TestEndpoint"};
};

class TestDataViewerCollection : public DataViewerCollection
{
   public:

    using DataViewerCollection::DispatchDataViewerEvent;
    using DataViewerCollection::IsViewerEnabled;
    using DataViewerCollection::IsViewerRegistered;
    using DataViewerCollection::RegisterViewer;
    using DataViewerCollection::UnregisterAllViewers;
    using DataViewerCollection::UnregisterViewer;

    std::vector<std::shared_ptr<IDataViewer>>& GetCollection()
    {
        return m_dataViewerCollection;
    }
};

TEST(DataViewerCollectionTests, RegisterViewer_DataViewerIsNullptr_ThrowsInvalidArgumentException)
{
    TestDataViewerCollection dataViewerCollection { };
    CheckForExceptionOrAbort<std::invalid_argument>([&dataViewerCollection]() { dataViewerCollection.RegisterViewer(nullptr); });
}

TEST(DataViewerCollectionTests, RegisterViewer_DataViewerIsNotNullptr_NoExceptions)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("MockViewer", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));
}

TEST(DataViewerCollectionTests, RegisterViewer_SharedDataViewerRegistered_SharedDataViewerRegisteredCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ true);
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));
    ASSERT_TRUE(dataViewerCollection.IsViewerRegistered(viewer->GetName()));
}

TEST(DataViewerCollectionTests, RegisterViewer_MultiplesharedDataViewersRegistered_sharedDataViewersRegisteredCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer4 = std::make_shared<MockIDataViewer>("sharedName4", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };

    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer1));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer2));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer3));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer4));

    ASSERT_EQ(dataViewerCollection.GetCollection().size(), size_t { 4 });
    ASSERT_TRUE(dataViewerCollection.IsViewerRegistered(viewer1->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerRegistered(viewer2->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerRegistered(viewer3->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerRegistered(viewer4->GetName()));
}

TEST(DataViewerCollectionTests, RegisterViewer_DuplicateDataViewerRegistered_ThrowsInvalidArgumentException)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));

    std::shared_ptr<IDataViewer> otherViewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    CheckForExceptionOrAbort<std::invalid_argument>([&dataViewerCollection, &otherViewer]() { dataViewerCollection.RegisterViewer(otherViewer); });
}

TEST(DataViewerCollectionTests, UnregisterViewer_ViewerNameIsNullPtr_ThrowsInvalidArgumentException)
{
    TestDataViewerCollection dataViewerCollection { };
    CheckForExceptionOrAbort<std::invalid_argument>([&dataViewerCollection]() { dataViewerCollection.UnregisterViewer(nullptr); });
}

TEST(DataViewerCollectionTests, UnregisterViewer_ViewerNameIsNotRegistered_ThrowsInvalidArgumentException)
{
    TestDataViewerCollection dataViewerCollection { };
    CheckForExceptionOrAbort<std::invalid_argument>([&dataViewerCollection]() { dataViewerCollection.UnregisterViewer("NotRegisteredViewer"); });
}

TEST(DataViewerCollectionTests, UnregisterViewer_ViewerNameIsRegistered_UnregistersCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);

    ASSERT_NO_THROW(dataViewerCollection.UnregisterViewer(viewer->GetName()));
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionTests, UnregisterViewer_ViewerNameMatchesByValue_UnregistersCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);

    // An equal name held at a different address: the collection must match on the characters,
    // not on the pointer the viewer happens to return from GetName().
    const std::string equalName { "sharedName" };
    ASSERT_NE(equalName.c_str(), viewer->GetName());

    ASSERT_NO_THROW(dataViewerCollection.UnregisterViewer(equalName.c_str()));
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionTests, UnregisterAllViewers_NoViewersRegistered_UnregisterCallSuccessful)
{
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
}

TEST(DataViewerCollectionTests, UnregisterAllViewers_OneViewerRegistered_UnregisterCallSuccessful)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);

    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionTests, UnregisterAllViewers_ThreeViewersRegistered_UnregisterCallSuccessful)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);

    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionTests, IsViewerEnabled_ViewerNameIsNullptr_ThrowInvalidArgumentException)
{
    TestDataViewerCollection dataViewerCollection { };
    CheckForExceptionOrAbort<std::invalid_argument>([&dataViewerCollection]() { dataViewerCollection.IsViewerEnabled(nullptr); });
}

TEST(DataViewerCollectionTests, IsViewerEnabled_NoViewerIsRegistered_ReturnsFalseCorrectly)
{
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled("sharedName"));
}

TEST(DataViewerCollectionTests, IsViewerEnabled_SingleViewerIsRegisteredAndNotTransmitting_ReturnsFalseCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled(viewer->GetName()));
}

TEST(DataViewerCollectionTests, IsViewerEnabled_SingleViewerIsRegisteredAndIsTransmitting_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ true);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled(viewer->GetName()));
}

TEST(DataViewerCollectionTests, IsViewerEnabled_MultipleViewersRegisteredAndNoneTransmitting_ReturnsFalseCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);

    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled("sharedName3"));
}

TEST(DataViewerCollectionTests, IsViewerEnabled_MultipleViewersRegisteredAndOneTransmitting_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ true);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);

    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled("sharedName1"));
}

TEST(DataViewerCollectionTests, IsViewerEnabledNoParam_NoViewerIsRegistered_ReturnsFalseCorrectly)
{
    TestDataViewerCollection dataViewerCollection { };
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionTests, IsViewerEnabledNoParam_SingleViewerIsRegisteredAndNotTransmitting_ReturnsFalseCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionTests, IsViewerEnabledNoParam_SingleViewerIsRegisteredAndIsTransmitting_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName", /*isTransmissionEnabled*/ true);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionTests, IsViewerEnabledNoParam_MultipleViewersRegisteredAndOneTransmitting_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ false);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ true);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ false);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionTests, IsViewerEnabledNoParam_MultipleViewersRegisteredAndAllTransmitting_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1", /*isTransmissionEnabled*/ true);
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2", /*isTransmissionEnabled*/ true);
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3", /*isTransmissionEnabled*/ true);
    TestDataViewerCollection dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}

namespace
{
    // Mirrors a viewer that reenters the SDK from its own callback - for example a Java
    // viewer that closes the owning LogManager from receiveData(), which unregisters every
    // viewer. m_dataViewerMapLock is recursive, so the reentrant call is admitted while
    // dispatch is still walking the collection.
    class ReentrantUnregisteringDataViewer : public IDataViewer
    {
       public:

        ReentrantUnregisteringDataViewer(const char* name, TestDataViewerCollection& collection) :
            m_name(name), m_collection(collection) {}

        void ReceiveData(const std::vector<uint8_t>&) noexcept override
        {
            callCount++;
            m_collection.UnregisterAllViewers();
        }

        const char* GetName() const noexcept override
        {
            return m_name;
        }

        bool IsTransmissionEnabled() const noexcept override
        {
            return true;
        }

        const std::string& GetCurrentEndpoint() const noexcept override
        {
            return m_testEndpoint;
        }

        int callCount { 0 };
        const char* m_name;
        TestDataViewerCollection& m_collection;
        const std::string m_testEndpoint { "TestEndpoint" };
    };
}

TEST(DataViewerCollectionTests, DispatchDataViewerEvent_ViewerUnregistersAllFromCallback_DispatchCompletesSafely)
{
    TestDataViewerCollection dataViewerCollection { };
    auto reentrantViewer = std::make_shared<ReentrantUnregisteringDataViewer>("ReentrantViewer", dataViewerCollection);
    auto secondViewer = std::make_shared<MockIDataViewer>("SecondViewer", /*isTransmissionEnabled*/ true);

    dataViewerCollection.RegisterViewer(reentrantViewer);
    dataViewerCollection.RegisterViewer(secondViewer);

    const std::vector<uint8_t> packetData { 1, 2, 3 };

    // Dispatching over the member vector directly would erase it mid-iteration here and
    // invalidate the iterator; dispatching over a snapshot completes and still delivers the
    // in-flight packet to viewers that were registered when dispatch began.
    dataViewerCollection.DispatchDataViewerEvent(packetData);

    ASSERT_EQ(reentrantViewer->callCount, 1);
    ASSERT_EQ(secondViewer->localPacketData, packetData);
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionTests, DispatchDataViewerEvent_MixedEnabledAndDisabledViewers_DispatchesOnlyToEnabled)
{
    TestDataViewerCollection dataViewerCollection { };
    auto enabledViewer = std::make_shared<MockIDataViewer>("EnabledViewer", /*isTransmissionEnabled*/ true);
    auto disabledViewer = std::make_shared<MockIDataViewer>("DisabledViewer", /*isTransmissionEnabled*/ false);

    dataViewerCollection.RegisterViewer(enabledViewer);
    dataViewerCollection.RegisterViewer(disabledViewer);

    const std::vector<uint8_t> packetData { 1, 2, 3 };
    dataViewerCollection.DispatchDataViewerEvent(packetData);

    // Gating is per viewer: one enabled viewer must not cause delivery to a disabled one.
    ASSERT_EQ(enabledViewer->localPacketData, packetData);
    ASSERT_TRUE(disabledViewer->localPacketData.empty());
}

TEST(DataViewerCollectionTests, DispatchDataViewerEvent_NoViewerEnabled_DispatchesToNobody)
{
    TestDataViewerCollection dataViewerCollection { };
    auto firstViewer = std::make_shared<MockIDataViewer>("FirstViewer", /*isTransmissionEnabled*/ false);
    auto secondViewer = std::make_shared<MockIDataViewer>("SecondViewer", /*isTransmissionEnabled*/ false);

    dataViewerCollection.RegisterViewer(firstViewer);
    dataViewerCollection.RegisterViewer(secondViewer);

    dataViewerCollection.DispatchDataViewerEvent(std::vector<uint8_t> { 1, 2, 3 });

    ASSERT_TRUE(firstViewer->localPacketData.empty());
    ASSERT_TRUE(secondViewer->localPacketData.empty());
}

namespace
{
    // Parks inside ReceiveData until released, so a test can observe the collection while a
    // viewer callback is genuinely in progress.
    class BlockingDataViewer : public IDataViewer
    {
       public:

        explicit BlockingDataViewer(const char* name) : m_name(name) {}

        void ReceiveData(const std::vector<uint8_t>&) noexcept override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_inCallback = true;
            m_entered.notify_all();
            m_release.wait(lock, [this] { return m_released; });
            m_inCallback = false;
        }

        const char* GetName() const noexcept override
        {
            return m_name;
        }

        bool IsTransmissionEnabled() const noexcept override
        {
            return true;
        }

        const std::string& GetCurrentEndpoint() const noexcept override
        {
            return m_testEndpoint;
        }

        void WaitUntilInCallback()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_entered.wait(lock, [this] { return m_inCallback; });
        }

        bool IsInCallback()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_inCallback;
        }

        void Release()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_released = true;
            }
            m_release.notify_all();
        }

       private:
        std::mutex m_mutex;
        std::condition_variable m_entered;
        std::condition_variable m_release;
        bool m_inCallback { false };
        bool m_released { false };
        const char* m_name;
        const std::string m_testEndpoint { "TestEndpoint" };
    };
}

TEST(DataViewerCollectionTests, UnregisterViewer_CallbackInProgress_ReturnsWithoutWaitingForCallback)
{
    TestDataViewerCollection dataViewerCollection { };
    auto blockingViewer = std::make_shared<BlockingDataViewer>("BlockingViewer");
    dataViewerCollection.RegisterViewer(blockingViewer);

    std::thread dispatcher([&dataViewerCollection]()
        {
            dataViewerCollection.DispatchDataViewerEvent(std::vector<uint8_t> { 1, 2, 3 });
        });

    blockingViewer->WaitUntilInCallback();

    // Unregistering must not wait for a callback already in progress. Making it wait would
    // deadlock any consumer whose callback cannot finish until the unregistering thread does.
    auto unregistered = std::async(std::launch::async, [&dataViewerCollection]()
        {
            dataViewerCollection.UnregisterViewer("BlockingViewer");
        });

    const auto unregisterStatus = unregistered.wait_for(std::chrono::seconds(30));

    // The witness: the viewer is still parked, so unregister genuinely returned early rather
    // than racing a callback that had already completed.
    const bool stillInCallback = blockingViewer->IsInCallback();
    const bool collectionEmptied = dataViewerCollection.GetCollection().empty();

    // Release before asserting so a regression fails the test instead of hanging the run.
    blockingViewer->Release();
    dispatcher.join();
    unregistered.get();

    ASSERT_EQ(unregisterStatus, std::future_status::ready) << "UnregisterViewer blocked while a viewer callback was in progress";
    ASSERT_TRUE(stillInCallback);
    ASSERT_TRUE(collectionEmptied);
}


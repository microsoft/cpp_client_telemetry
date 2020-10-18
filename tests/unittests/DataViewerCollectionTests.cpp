//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#include "common/Common.hpp"
#include "api/DataViewerCollection.hpp"
#include "CheckForExceptionOrAbort.hpp"

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


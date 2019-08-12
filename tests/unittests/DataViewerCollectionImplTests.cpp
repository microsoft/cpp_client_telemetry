// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/DataViewerCollectionImpl.hpp"

using namespace testing;
using namespace MAT;

class MockIDataViewer : public IDataViewer
{
public:
    MockIDataViewer()
        : MockIDataViewer("MockIDataViewer") {}
    
    MockIDataViewer(const char* name)
        : m_name(name) {}

    void RecieveData(const std::vector<std::uint8_t>& packetData) noexcept override
    {
        localPacketData.assign(packetData.cbegin(), packetData.cend() );
    }

    const char* const GetName() const noexcept override
    {
        return m_name;
    }

    std::vector<std::uint8_t> localPacketData;
    const char* m_name;
};

class TestDataViewerCollectionImpl : public DataViewerCollectionImpl
{
public:

   using DataViewerCollectionImpl::DispatchDataViewerEvent;
   using DataViewerCollectionImpl::RegisterViewer;
   using DataViewerCollectionImpl::UnregisterViewer;
   using DataViewerCollectionImpl::UnregisterAllViewers;
   using DataViewerCollectionImpl::IsViewerEnabled;

   std::map<const char*, std::unique_ptr<IDataViewer>>& GetCollection()
   {
       return m_dataViewerCollection;
   }
};

TEST(DataViewerCollectionImplTests, RegisterViewer_DataViewerIsNullptr_ThrowsInvalidArgumentException)
{
     TestDataViewerCollectionImpl dataViewerCollection { };
     ASSERT_THROW(dataViewerCollection.RegisterViewer(nullptr), std::invalid_argument);
}

TEST(DataViewerCollectionImplTests, RegisterViewer_DataViewerIsNotNullptr_NoExceptions)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>();
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer)));
}

TEST(DataViewerCollectionImplTests, RegisterViewer_UniqueDataViewerRegistered_UniqueDataViewerRegisteredCorrectly)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer)));
    ASSERT_NE(dataViewerCollection.GetCollection().find("UniqueName"), dataViewerCollection.GetCollection().end());
}

TEST(DataViewerCollectionImplTests, RegisterViewer_MultipleUniqueDataViewersRegistered_UniqueDataViewersRegisteredCorrectly)
{
    std::unique_ptr<IDataViewer> viewer1 = std::make_unique<MockIDataViewer>("UniqueName1");
    std::unique_ptr<IDataViewer> viewer2 = std::make_unique<MockIDataViewer>("UniqueName2");
    std::unique_ptr<IDataViewer> viewer3 = std::make_unique<MockIDataViewer>("UniqueName3");
    std::unique_ptr<IDataViewer> viewer4 = std::make_unique<MockIDataViewer>("UniqueName4");
    TestDataViewerCollectionImpl dataViewerCollection { };

    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer1)));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer2)));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer3)));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer4)));

    ASSERT_EQ(dataViewerCollection.GetCollection().size(), 4);
    ASSERT_NE(dataViewerCollection.GetCollection().find("UniqueName1"), dataViewerCollection.GetCollection().end());
    ASSERT_NE(dataViewerCollection.GetCollection().find("UniqueName2"), dataViewerCollection.GetCollection().end());
    ASSERT_NE(dataViewerCollection.GetCollection().find("UniqueName3"), dataViewerCollection.GetCollection().end());
    ASSERT_NE(dataViewerCollection.GetCollection().find("UniqueName4"), dataViewerCollection.GetCollection().end());
}

TEST(DataViewerCollectionImplTests, RegisterViewer_DuplicateDataViewerRegistered_ThrowsInvalidArgumentException)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(std::move(viewer)));

    viewer = std::make_unique<MockIDataViewer>("UniqueName");
    ASSERT_THROW(dataViewerCollection.RegisterViewer(std::move(viewer)), std::invalid_argument);
}

TEST(DataViewerCollectionImplTests, UnregisterViewer_ViewerNameIsNullPtr_ThrowsInvalidArgumentException)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_THROW(dataViewerCollection.UnregisterViewer(nullptr), std::invalid_argument);
}

TEST(DataViewerCollectionImplTests, UnregisterViewer_ViewerNameIsNotRegistered_ThrowsInvalidArgumentException)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_THROW(dataViewerCollection.UnregisterViewer("NotRegisteredViewer"), std::invalid_argument);
}

TEST(DataViewerCollectionImplTests, UnregisterViewer_ViewerNameIsRegistered_UnregistersCorrectly)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName", std::move(viewer));
    ASSERT_NO_THROW(dataViewerCollection.UnregisterViewer("UniqueName"));
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_NoViewersRegistered_UnregisterCallSuccessful)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_OneViewerRegistered_UnregisterCallSuccessful)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName", std::move(viewer));
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_ThreeViewersRegistered_UnregisterCallSuccessful)
{
    std::unique_ptr<IDataViewer> viewer1 = std::make_unique<MockIDataViewer>("UniqueName1");
    std::unique_ptr<IDataViewer> viewer2 = std::make_unique<MockIDataViewer>("UniqueName2");
    std::unique_ptr<IDataViewer> viewer3 = std::make_unique<MockIDataViewer>("UniqueName3");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName1", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName2", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName3", std::move(viewer1));
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_ViewerNameIsNullptr_ThrowInvalidArgumentException)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_THROW(dataViewerCollection.IsViewerEnabled(nullptr), std::invalid_argument);
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_NoViewerIsRegistered_ReturnsFalseCorrectly)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled("UniqueName"));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_SingleViewerIsRegistered_ReturnsTrueCorrectly)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName", std::move(viewer));
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled("UniqueName"));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_MultipleViewersRegistered_ReturnsTrueCorrectly)
{
    std::unique_ptr<IDataViewer> viewer1 = std::make_unique<MockIDataViewer>("UniqueName1");
    std::unique_ptr<IDataViewer> viewer2 = std::make_unique<MockIDataViewer>("UniqueName2");
    std::unique_ptr<IDataViewer> viewer3 = std::make_unique<MockIDataViewer>("UniqueName3");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName1", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName2", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName3", std::move(viewer1));
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled("UniqueName3"));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_NoViewerIsRegistered_ReturnsFalseCorrectly)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_SingleViewerIsRegistered_ReturnsTrueCorrectly)
{
    std::unique_ptr<IDataViewer> viewer = std::make_unique<MockIDataViewer>("UniqueName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName", std::move(viewer));
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_MultipleViewersRegistered_ReturnsTrueCorrectly)
{
    std::unique_ptr<IDataViewer> viewer1 = std::make_unique<MockIDataViewer>("UniqueName1");
    std::unique_ptr<IDataViewer> viewer2 = std::make_unique<MockIDataViewer>("UniqueName2");
    std::unique_ptr<IDataViewer> viewer3 = std::make_unique<MockIDataViewer>("UniqueName3");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().emplace("UniqueName1", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName2", std::move(viewer1));
    dataViewerCollection.GetCollection().emplace("UniqueName3", std::move(viewer1));
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}
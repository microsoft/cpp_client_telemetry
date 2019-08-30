// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/DataViewerCollection.hpp"

using namespace testing;
using namespace MAT;

class MockIDataViewer : public IDataViewer
{
public:
    MockIDataViewer()
        : MockIDataViewer("MockIDataViewer") {}
    
    MockIDataViewer(const char* name)
        : m_name(name) {}

    void RecieveData(const std::vector<std::uint8_t>& packetData) const noexcept override
    {
        localPacketData = packetData;
    }

    const char* const GetName() const noexcept override
    {
        return m_name;
    }

    mutable std::vector<std::uint8_t> localPacketData;
    const char* m_name;
};

class TestDataViewerCollectionImpl : public DataViewerCollection
{
public:

   using DataViewerCollection::DispatchDataViewerEvent;
   using DataViewerCollection::RegisterViewer;
   using DataViewerCollection::UnregisterViewer;
   using DataViewerCollection::UnregisterAllViewers;
   using DataViewerCollection::IsViewerEnabled;
   using DataViewerCollection::IsViewerInCollection;

   std::vector<std::shared_ptr<IDataViewer>>& GetCollection()
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
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>();
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));
}

TEST(DataViewerCollectionImplTests, RegisterViewer_sharedDataViewerRegistered_sharedDataViewerRegisteredCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));
    ASSERT_TRUE(dataViewerCollection.IsViewerInCollection(viewer->GetName()));
}

TEST(DataViewerCollectionImplTests, RegisterViewer_MultiplesharedDataViewersRegistered_sharedDataViewersRegisteredCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1");
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2");
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3");
    std::shared_ptr<IDataViewer> viewer4 = std::make_shared<MockIDataViewer>("sharedName4");
    TestDataViewerCollectionImpl dataViewerCollection { };

    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer1));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer2));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer3));
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer4));

    ASSERT_EQ(dataViewerCollection.GetCollection().size(), 4);
    ASSERT_TRUE(dataViewerCollection.IsViewerInCollection(viewer1->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerInCollection(viewer2->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerInCollection(viewer3->GetName()));
    ASSERT_TRUE(dataViewerCollection.IsViewerInCollection(viewer4->GetName()));
}

TEST(DataViewerCollectionImplTests, RegisterViewer_DuplicateDataViewerRegistered_ThrowsInvalidArgumentException)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.RegisterViewer(viewer));

    viewer = std::make_shared<MockIDataViewer>("sharedName");
    ASSERT_THROW(dataViewerCollection.RegisterViewer(viewer), std::invalid_argument);
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
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");
    
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);

    ASSERT_NO_THROW(dataViewerCollection.UnregisterViewer(viewer->GetName()));
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_NoViewersRegistered_UnregisterCallSuccessful)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_OneViewerRegistered_UnregisterCallSuccessful)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");

    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    
    ASSERT_NO_THROW(dataViewerCollection.UnregisterAllViewers());
    ASSERT_TRUE(dataViewerCollection.GetCollection().empty());
}

TEST(DataViewerCollectionImplTests, UnregisterAllViewers_ThreeViewersRegistered_UnregisterCallSuccessful)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1");
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2");
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3");
    
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);

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
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled("sharedName"));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_SingleViewerIsRegistered_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled(viewer->GetName()));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabled_MultipleViewersRegistered_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1");
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2");
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3");
    
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);
    
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled("sharedName3"));
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_NoViewerIsRegistered_ReturnsFalseCorrectly)
{
    TestDataViewerCollectionImpl dataViewerCollection { };
    ASSERT_FALSE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_SingleViewerIsRegistered_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer = std::make_shared<MockIDataViewer>("sharedName");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}

TEST(DataViewerCollectionImplTests, IsViewerEnabledNoParam_MultipleViewersRegistered_ReturnsTrueCorrectly)
{
    std::shared_ptr<IDataViewer> viewer1 = std::make_shared<MockIDataViewer>("sharedName1");
    std::shared_ptr<IDataViewer> viewer2 = std::make_shared<MockIDataViewer>("sharedName2");
    std::shared_ptr<IDataViewer> viewer3 = std::make_shared<MockIDataViewer>("sharedName3");
    TestDataViewerCollectionImpl dataViewerCollection { };
    dataViewerCollection.GetCollection().push_back(viewer1);
    dataViewerCollection.GetCollection().push_back(viewer2);
    dataViewerCollection.GetCollection().push_back(viewer3);
    ASSERT_TRUE(dataViewerCollection.IsViewerEnabled());
}
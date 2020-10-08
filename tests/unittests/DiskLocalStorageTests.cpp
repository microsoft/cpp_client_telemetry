#if 0
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"

//#include "controlplane/DiskLocalStorage.hpp"

#include "common/MockITenantDataSerializer.hpp"

using namespace testing;
using namespace MAT;

//using namespace MAT::ControlPlane;

// Class to allow unit tests to test DiskLocalStorage without needing to use the file system
class TestableDiskLocalStorage : public DiskLocalStorage
{
    std::istream* m_streamToReturn;

    std::istream * OpenStream(const std::string& pathToFile) const override
    {
        UNREFERENCED_PARAMETER(pathToFile);
        return m_streamToReturn;
    }

public:
    TestableDiskLocalStorage(const std::string& pathToCache, std::unique_ptr<ITenantDataSerializer> &serializer,
        std::istream* streamToReturn)
        : DiskLocalStorage(pathToCache, serializer), m_streamToReturn(streamToReturn)
    {
    }
};

TEST(DiskLocalStorageTests, Ctor_Succeeds)
{
    MockITenantDataSerializer* serializerMock = new StrictMock<MockITenantDataSerializer>();
    std::unique_ptr<ITenantDataSerializer> serializer(serializerMock);
    DiskLocalStorage reader("", serializer);
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamFails_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    MockITenantDataSerializer* serializerMock = new StrictMock<MockITenantDataSerializer>();
    std::unique_ptr<ITenantDataSerializer> serializer(serializerMock);

    TestableDiskLocalStorage reader(path, serializer, nullptr);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsEmptyStream_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    MockITenantDataSerializer* serializerMock = new StrictMock<MockITenantDataSerializer>();
    std::unique_ptr<ITenantDataSerializer> serializer(serializerMock);
    std::string expectedInput = "";
    std::istream * testStream = new std::istringstream(expectedInput);

    TestableDiskLocalStorage reader(path, serializer, testStream);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsNonEmptyStream_SerializerReturnsNullPtr_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    MockITenantDataSerializer* serializerMock = new StrictMock<MockITenantDataSerializer>();
    std::unique_ptr<ITenantDataSerializer> serializer(serializerMock);
    std::string expectedInput = "This should return nullptr";
    std::istream * testStream = new std::istringstream(expectedInput);

    std::string actualInput;

    EXPECT_CALL(*serializerMock, DeserializeTenantData(_))
        .WillOnce(DoAll(SaveArg<0>(&actualInput),Return(nullptr)));
    TestableDiskLocalStorage reader(path, serializer, testStream);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
    ASSERT_EQ(expectedInput, actualInput);
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsNonEmptyStream_SerializerReturnsTenantDataPtr_ReturnsTenantDataPtr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    MockITenantDataSerializer* serializerMock = new StrictMock<MockITenantDataSerializer>();
    std::unique_ptr<ITenantDataSerializer> serializer(serializerMock);
    std::string expectedInput = "This should return TenantDataPtr";
    std::istream * testStream = new std::istringstream(expectedInput);

    std::string actualInput;

    TenantData tenantData;

    EXPECT_CALL(*serializerMock, DeserializeTenantData(_))
        .WillOnce(DoAll(SaveArg<0>(&actualInput), Return(&tenantData)));
    TestableDiskLocalStorage reader(path, serializer, testStream);
    ASSERT_EQ(&tenantData, reader.ReadTenantData(ariaTenantId));
    ASSERT_EQ(expectedInput, actualInput);
}
#endif


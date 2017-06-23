// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "controlplane/DiskLocalStorage.hpp"
#include "common/MockITenantDataSerializer.hpp"

using namespace testing;
using namespace ARIASDK_NS;
using namespace ARIASDK_NS::ControlPlane;

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
    TestableDiskLocalStorage(const std::string& pathToCache, ITenantDataSerializer& serializer,
        std::istream* streamToReturn)
        : DiskLocalStorage(pathToCache, serializer), m_streamToReturn(streamToReturn)
    {
    }
};

TEST(DiskLocalStorageTests, Ctor_Succeeds)
{
    StrictMock<MockITenantDataSerializer> mockISerializer;
    DiskLocalStorage reader("", mockISerializer);
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamFails_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;

    StrictMock<MockITenantDataSerializer> mockISerializer;
    TestableDiskLocalStorage reader(path, mockISerializer, nullptr);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsEmptyStream_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    std::string expectedInput = "";
    std::istream * testStream = new std::istringstream(expectedInput);

    StrictMock<MockITenantDataSerializer> mockISerializer;
    TestableDiskLocalStorage reader(path, mockISerializer, testStream);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsNonEmptyStream_SerializerReturnsNullPtr_ReturnsNullptr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    std::string expectedInput = "This should return nullptr";
    std::istream * testStream = new std::istringstream(expectedInput);

    std::string actualInput;

    StrictMock<MockITenantDataSerializer> mockISerializer;
    EXPECT_CALL(mockISerializer, DeserializeTenantData(_))
        .WillOnce(DoAll(SaveArg<0>(&actualInput),Return(nullptr)));
    TestableDiskLocalStorage reader(path, mockISerializer, testStream);
    ASSERT_EQ(nullptr, reader.ReadTenantData(ariaTenantId));
    ASSERT_EQ(expectedInput, actualInput);
}

TEST(DiskLocalStorageTests, ReadTenantData_OpenStreamReturnsNonEmptyStream_SerializerReturnsTenantDataPtr_ReturnsTenantDataPtr)
{
    LPCSTR path = "My Path";
    GUID_t ariaTenantId;
    std::string expectedInput = "This should return TenantDataPtr";
    std::istream * testStream = new std::istringstream(expectedInput);

    std::string actualInput;

    TenantData tenantData;

    StrictMock<MockITenantDataSerializer> mockISerializer;
    EXPECT_CALL(mockISerializer, DeserializeTenantData(_))
        .WillOnce(DoAll(SaveArg<0>(&actualInput), Return(&tenantData)));
    TestableDiskLocalStorage reader(path, mockISerializer, testStream);
    ASSERT_EQ(&tenantData, reader.ReadTenantData(ariaTenantId));
    ASSERT_EQ(expectedInput, actualInput);
}

#if 0
// Copyright (c) Microsoft. All rights reserved.

#include <exception>
#include "common/Common.hpp"
#include "common/MockILocalStorageReader.hpp"
#include "controlplane/SingleControlPlane.hpp"

using namespace testing;
using namespace MAT_NS;
using namespace MAT_NS::ControlPlane;

TEST(SingleControlPlaneTests, ValidStorageConstructsAndDestructs)
{
    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));
}

TEST(SingleControlPlaneTests, GetStringParameter_NoTenantDataExists_ReturnsDefaultValue)
{
    GUID_t testGuid;
    const std::string defaultValue = "myDefault";
    const std::string key = "Superman";

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetStringParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsDefaultValue)
{
    GUID_t testGuid;
    const std::string defaultValue = "myDefault";
    const std::string key = "Superman";

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetStringParameter_TenantDataExists_ParameterIsInDictionary_ReturnsDictionaryValue)
{
    GUID_t testGuid;
    const std::string defaultValue = "myDefault";
    const std::string key = "Superman";
    const std::string value = "Metropolis";

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_stringMap[key] = value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(value, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
    ASSERT_EQ(value, *controlPlane->GetStringParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetLongParameter_NoTenantDataExists_ReturnsDefaultValue)
{
    GUID_t testGuid;
    long defaultValue = 4;
    const std::string key = "Justice League";

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, controlPlane->GetLongParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, controlPlane->GetLongParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetLongParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsDefaultValue)
{
    GUID_t testGuid;
    long defaultValue = 4;
    const std::string key = "Justice League";

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, controlPlane->GetLongParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, controlPlane->GetLongParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetLongParameter_TenantDataExists_ParameterIsInDictionary_ReturnsDictionaryValue)
{
    GUID_t testGuid;
    long defaultValue = 4;
    const std::string key = "Justice League";
    long value = 5;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_longMap[key] = value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(value, controlPlane->GetLongParameter(testGuid, key, defaultValue));
    ASSERT_EQ(value, controlPlane->GetLongParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetBoolParameter_NoTenantDataExists_ReturnsDefaultValue)
{
    GUID_t testGuid;
    bool defaultValue = false;
    const std::string key = "Kryptonite";

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetBoolParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsDefaultValue)
{
    GUID_t testGuid;
    bool defaultValue = true;
    const std::string key = "Kryptonite";

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(defaultValue, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
    ASSERT_EQ(defaultValue, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, GetBoolParameter_TenantDataExists_ParameterIsInDictionary_ReturnsDictionaryValue)
{
    GUID_t testGuid;
    bool defaultValue = false;
    const std::string key = "Kryptonite";
    bool value = true;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_boolMap[key] = value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_EQ(value, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
    ASSERT_EQ(value, controlPlane->GetBoolParameter(testGuid, key, defaultValue));
}

TEST(SingleControlPlaneTests, TryGetStringParameter_NoTenantDataExists_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Superman";
    std::string value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetStringParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetStringParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, TryGetStringParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Superman";
    std::string value;

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetStringParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetStringParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, TryGetStringParameter_TenantDataExists_ParameterIsInDictionary_ReturnsTrue_ValueIsCorrect)
{
    GUID_t testGuid;
    const std::string key = "Superman";
    const std::string expectedValue = "Metropolis";
    std::string value;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_stringMap[key] = expectedValue;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
}

TEST(SingleControlPlaneTests, TryGetLongParameter_NoTenantDataExists_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Justice League";
    long value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetLongParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetLongParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, TryGetLongParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Justice Leage";
    long value;

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetLongParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetLongParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, TryGetLongParameter_TenantDataExists_ParameterIsInDictionary_ReturnsTrue_ValueIsCorrect)
{
    GUID_t testGuid;
    const std::string key = "Justice Leage";
    long expectedValue = 5;
    long value;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_longMap[key] = expectedValue;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetLongParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
    ASSERT_TRUE(controlPlane->TryGetLongParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
}

TEST(SingleControlPlaneTests, TryGetBoolParameter_NoTenantDataExists_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Kryptonite";
    bool value;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(_))
        .WillOnce(Return(nullptr));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetBoolParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, GetBoolParameter_TenantDataExists_ParameterIsNotInDictionary_ReturnsFalse)
{
    GUID_t testGuid;
    const std::string key = "Kryptonite";
    bool value;

    TenantData tenantData;
    tenantData.m_isDummy = false;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_FALSE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_FALSE(controlPlane->TryGetBoolParameter(testGuid, key, value));
}

TEST(SingleControlPlaneTests, TryGetBoolParameter_TenantDataExists_ParameterIsInDictionary_ReturnsTrue_ValueIsCorrect)
{
    GUID_t testGuid;
    const std::string key = "Kryptonite";
    bool expectedValue = true;
    bool value;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_boolMap[key] = expectedValue;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedValue, value);
}

TEST(SingleControlPlaneTests, TryGetStringParameter_TwoTenantsWithConflictingData_CorrectDataIsReadFromEachTenant)
{
    GUID_t testGuid1("{25D580F6-6532-41B1-B7E5-09D9E2C19697}");
    GUID_t testGuid2("{82D1891A-C4E5-4324-BC00-19ECF50D68FC}");
    const std::string key = "Best Universe";
    const std::string expectedValue1 = "DC";
    const std::string expectedValue2 = "Marvel";

    std::string value;

    TenantData tenantData1;
    tenantData1.m_isDummy = false;
    tenantData1.m_stringMap[key] = expectedValue1;

    TenantData tenantData2;
    tenantData2.m_isDummy = false;
    tenantData2.m_stringMap[key] = expectedValue2;

    MockILocalStorageReader* readerMock = new StrictMock<MockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid1))
        .WillOnce(Return(&tenantData1));
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid2))
        .WillOnce(Return(&tenantData2));

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid1, key, value));
    ASSERT_EQ(expectedValue1, value);
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid2, key, value));
    ASSERT_EQ(expectedValue2, value);
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid1, key, value));
    ASSERT_EQ(expectedValue1, value);
    ASSERT_TRUE(controlPlane->TryGetStringParameter(testGuid2, key, value));
    ASSERT_EQ(expectedValue2, value);
}

TEST(SingleControlPlaneTests, OnChange_TenantHasNotBeenRead_DataDoesNotRefresh)
{
    GUID_t testGuid;
    const std::string key = "Kryptonite";
    bool expectedTrue = true;
    bool value;

    TenantData tenantData;
    tenantData.m_isDummy = false;
    tenantData.m_boolMap[key] = expectedTrue;

    NotifiableMockILocalStorageReader* readerMock = new StrictMock<NotifiableMockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantData))
        .RetiresOnSaturation();

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedTrue, value);
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedTrue, value);

    // Invoke the change handler with an unknown tenantId
    GUID_t changeTenantId("{038C3CD7-82B8-422F-8E6E-47270F2D5A3A}");
    readerMock->m_handler->OnChange(*reader, changeTenantId);

    // Nothing should have changed here
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedTrue, value);
}

TEST(SingleControlPlaneTests, OnChange_TenantHasBeenRead_DataRefreshes)
{
    GUID_t testGuid;
    const std::string key = "Kryptonite";
    bool expectedTrue = true;
    bool expectedFalse = false;
    bool value;

    TenantData tenantDataTrue;
    tenantDataTrue.m_isDummy = false;
    tenantDataTrue.m_boolMap[key] = expectedTrue;

    TenantData tenantDataFalse;
    tenantDataFalse.m_isDummy = false;
    tenantDataFalse.m_boolMap[key] = expectedFalse;

    NotifiableMockILocalStorageReader* readerMock = new StrictMock<NotifiableMockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    // GMock uses sticky WillOnce values, so this will return first tenantDataTrue, then tenantDataFalse.
    // Reference: https://github.com/google/googletest/blob/master/googlemock/docs/ForDummies.md
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantDataFalse))
        .RetiresOnSaturation();
    EXPECT_CALL(*readerMock, ReadTenantData(testGuid))
        .WillOnce(Return(&tenantDataTrue))
        .RetiresOnSaturation();

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedTrue, value);
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedTrue, value);

    // Invoke the change handler
    readerMock->m_handler->OnChange(*reader, testGuid);

    // Call these twice, to ensure that the TenantDataPtr is properly cached
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedFalse, value);
    ASSERT_TRUE(controlPlane->TryGetBoolParameter(testGuid, key, value));
    ASSERT_EQ(expectedFalse, value);
}

class TestControlPlaneChangeEventHandler : public IControlPlaneChangeEventHandler
{
public:
    int m_count;
    IControlPlane* m_lastControlPlane;
    GUID_t m_lastAriaTenantId;

    void OnChange(IControlPlane& controlPlane, const GUID_t& ariaTenantId) override
    {
        m_lastControlPlane = &controlPlane;
        m_lastAriaTenantId = ariaTenantId;
        m_count++;
    }

    TestControlPlaneChangeEventHandler()
    {
        m_count = 0;
        m_lastControlPlane = nullptr;
    }
};

TEST(SingleControlPlaneTests, OnChange_ListenerIsRegistered_NotificationHappens)
{
    NotifiableMockILocalStorageReader* readerMock = new StrictMock<NotifiableMockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    TestControlPlaneChangeEventHandler handler;
    controlPlane->RegisterChangeEventHandler(&handler);

    GUID_t ariaTenantId1("{09FA7F18-82F1-49F6-8D1C-F58CA7942FAF}");
    GUID_t ariaTenantId2("{B3616130-CA91-4E13-BF1B-84F7D488F1AE}");

    ASSERT_EQ(0, handler.m_count);
    ASSERT_EQ(nullptr, handler.m_lastControlPlane);

    readerMock->m_handler->OnChange(*reader, ariaTenantId1);
    ASSERT_EQ(1, handler.m_count);
    ASSERT_EQ(controlPlane.get(), handler.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler.m_lastAriaTenantId);

    readerMock->m_handler->OnChange(*reader, ariaTenantId2);
    ASSERT_EQ(2, handler.m_count);
    ASSERT_EQ(controlPlane.get(), handler.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId2, handler.m_lastAriaTenantId);
}

TEST(SingleControlPlaneTests, OnChange_ListenerIsUnregistered_NotificationStops)
{
    NotifiableMockILocalStorageReader* readerMock = new StrictMock<NotifiableMockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    TestControlPlaneChangeEventHandler handler;
    controlPlane->RegisterChangeEventHandler(&handler);

    GUID_t ariaTenantId1("{09FA7F18-82F1-49F6-8D1C-F58CA7942FAF}");
    GUID_t ariaTenantId2("{B3616130-CA91-4E13-BF1B-84F7D488F1AE}");

    ASSERT_EQ(0, handler.m_count);
    ASSERT_EQ(nullptr, handler.m_lastControlPlane);

    readerMock->m_handler->OnChange(*reader, ariaTenantId1);
    ASSERT_EQ(1, handler.m_count);
    ASSERT_EQ(controlPlane.get(), handler.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler.m_lastAriaTenantId);

    controlPlane->UnregisterChangeEventHandler(&handler);
    readerMock->m_handler->OnChange(*reader, ariaTenantId2);
    ASSERT_EQ(1, handler.m_count);
    ASSERT_EQ(controlPlane.get(), handler.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler.m_lastAriaTenantId);
}

TEST(SingleControlPlaneTests, MultipleListenersAreRegistered_EachIsNotified)
{
    NotifiableMockILocalStorageReader* readerMock = new StrictMock<NotifiableMockILocalStorageReader>();
    std::unique_ptr<ILocalStorageReader> reader(readerMock);
    EXPECT_CALL(*readerMock, RegisterChangeEventHandler(_))
        .Times(1);
    EXPECT_CALL(*readerMock, UnregisterChangeEventHandler(_))
        .Times(1);

    std::unique_ptr<IControlPlane> controlPlane(new SingleControlPlane(reader));

    TestControlPlaneChangeEventHandler handler1;
    TestControlPlaneChangeEventHandler handler2;
    TestControlPlaneChangeEventHandler handler3;
    controlPlane->RegisterChangeEventHandler(&handler1);
    controlPlane->RegisterChangeEventHandler(&handler2);
    controlPlane->RegisterChangeEventHandler(&handler3);

    GUID_t ariaTenantId1("{09FA7F18-82F1-49F6-8D1C-F58CA7942FAF}");
    GUID_t ariaTenantId2("{B3616130-CA91-4E13-BF1B-84F7D488F1AE}");

    ASSERT_EQ(0, handler1.m_count);
    ASSERT_EQ(nullptr, handler1.m_lastControlPlane);
    ASSERT_EQ(0, handler2.m_count);
    ASSERT_EQ(nullptr, handler2.m_lastControlPlane);
    ASSERT_EQ(0, handler3.m_count);
    ASSERT_EQ(nullptr, handler3.m_lastControlPlane);

    readerMock->m_handler->OnChange(*reader, ariaTenantId1);
    ASSERT_EQ(1, handler1.m_count);
    ASSERT_EQ(controlPlane.get(), handler1.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler1.m_lastAriaTenantId);
    ASSERT_EQ(1, handler2.m_count);
    ASSERT_EQ(controlPlane.get(), handler2.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler2.m_lastAriaTenantId);
    ASSERT_EQ(1, handler3.m_count);
    ASSERT_EQ(controlPlane.get(), handler3.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler3.m_lastAriaTenantId);

    controlPlane->UnregisterChangeEventHandler(&handler2);
    readerMock->m_handler->OnChange(*reader, ariaTenantId2);
    ASSERT_EQ(2, handler1.m_count);
    ASSERT_EQ(controlPlane.get(), handler1.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId2, handler1.m_lastAriaTenantId);
    ASSERT_EQ(1, handler2.m_count);
    ASSERT_EQ(controlPlane.get(), handler2.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId1, handler2.m_lastAriaTenantId);
    ASSERT_EQ(2, handler3.m_count);
    ASSERT_EQ(controlPlane.get(), handler3.m_lastControlPlane);
    ASSERT_EQ(ariaTenantId2, handler3.m_lastAriaTenantId);
}
#endif
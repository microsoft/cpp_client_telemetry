//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"

#include "api/ContextFieldsProvider.hpp"
#include "utils/Utils.hpp"

#if !defined(_WIN32) && !defined(ANDROID)
#include "pal/posix/sysinfo_sources_impl.hpp"
#endif
#ifdef __APPLE__
#include "pal/posix/sysinfo_utils_apple.hpp"
#endif

using namespace MAT;

TEST(DeviceIdTests, NativeCollectorFollowsBuildOption)
{
    auto device = PAL::GetDeviceInformation();
    ASSERT_NE(device, nullptr);
#ifdef MATSDK_DISABLE_DEVICE_ID
    EXPECT_TRUE(device->GetDeviceId().empty());
#if !defined(_WIN32) && !defined(ANDROID)
    EXPECT_TRUE(sysinfo_sources_impl::GetSysInfo().get("devId").empty());
#endif
#ifdef __APPLE__
    EXPECT_TRUE(GetDeviceId().empty());
#endif
#else
    EXPECT_FALSE(device->GetDeviceId().empty());
#endif
}

TEST(DeviceIdTests, SemanticRegistrationPreservesCallerIdWhenCollectionIsDisabled)
{
    ContextFieldsProvider context;
    const std::string callerId = "c:" + std::string(8 * 1024, 'x');
    context.SetDeviceId(callerId);
    PAL::registerSemanticContext(&context);

    const auto& fields = context.GetCommonFields();
    ASSERT_EQ(fields.count(COMMONFIELDS_DEVICE_ID), 1u);
#ifdef MATSDK_DISABLE_DEVICE_ID
    const std::string expected = callerId;
#else
    const std::string expected = boundedSystemInfo(PAL::GetDeviceInformation()->GetDeviceId().c_str());
#endif
    EXPECT_EQ(std::string(fields.at(COMMONFIELDS_DEVICE_ID).as_string), expected);
#ifdef MATSDK_DISABLE_DEVICE_ID
    CsProtocol::Record record;
    context.writeToRecord(record);
    ASSERT_FALSE(record.extDevice.empty());
    EXPECT_EQ(record.extDevice[0].localId, expected);
#endif
}

TEST(DeviceIdTests, RegistrationRetainsOtherMetadataWithoutInventingAnId)
{
    ContextFieldsProvider context;
    PAL::registerSemanticContext(&context);
    const auto& fields = context.GetCommonFields();
#ifdef MATSDK_DISABLE_DEVICE_ID
    EXPECT_EQ(fields.count(COMMONFIELDS_DEVICE_ID), 0u);
#else
    EXPECT_EQ(fields.count(COMMONFIELDS_DEVICE_ID), 1u);
#endif
    ASSERT_EQ(fields.count(COMMONFIELDS_OS_NAME), 1u);
    ASSERT_EQ(fields.count(COMMONFIELDS_APP_ID), 1u);
    EXPECT_EQ(std::string(fields.at(COMMONFIELDS_OS_NAME).as_string),
              boundedSystemInfo(PAL::GetSystemInformation()->GetOsName().c_str()));
    EXPECT_EQ(std::string(fields.at(COMMONFIELDS_APP_ID).as_string),
              boundedSystemInfo(PAL::GetSystemInformation()->GetAppId().c_str()));
}

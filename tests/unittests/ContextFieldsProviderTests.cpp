// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "api/ContextFieldsProvider.hpp"

using namespace testing;
using namespace ARIASDK_NS;


TEST(ContextFieldsProviderTests, SetProperties)
{
    ContextFieldsProvider ctx(nullptr);
    ContextFieldsProvider loggerCtx(&ctx);

    ctx.setCustomField("shared", "willbeoverwritten");
    ctx.setCustomField("parent", "willremain");
    ctx.setCustomField("empty", "");
    ctx.setCustomField("parentonly", "willberemoved");
	EventProperty prop("willbeoverwrittenpii", PiiKind_DistinguishedName);
    ctx.setCustomField("sharedpii", prop);
	EventProperty prop1("willremainpii", PiiKind_GenericData);	
    ctx.setCustomField("parentpii", prop1);
	EventProperty prop2("", PiiKind_Identity);
    ctx.setCustomField("emptypii", prop2);
	EventProperty prop3("willberemoved", PiiKind_GenericData);
    ctx.setCustomField("parentonlypii", prop3 );

    ctx.SetAppId("appId");
    ctx.SetAppExperimentIds("appExperimentIds");
    ctx.SetAppLanguage("appLanguage");
    ctx.SetAppVersion("appVersion");

    ctx.SetDeviceId("deviceId");
    ctx.SetDeviceMake("deviceMake");
    ctx.SetDeviceModel("deviceModel");

    ctx.SetNetworkCost(NetworkCost_Unmetered);
    ctx.SetNetworkProvider("networkProvider");
    ctx.SetNetworkType(NetworkType_Wired);

    ctx.SetOsBuild("osBuild");
    ctx.SetOsName("osName");
    ctx.SetOsVersion("osVersion");

    ctx.SetUserId("userId", PiiKind_Identity);
    ctx.SetUserMsaId("userMsaId");
    ctx.SetUserANID("userANID");
    ctx.SetUserAdvertisingId("userAdvertingId");
    ctx.SetUserLanguage("language");
    ctx.SetUserTimeZone("timeZone");

    ::AriaProtocol::Record record;
    loggerCtx.writeToRecord(record);

    EXPECT_THAT(record.PIIExtensions.size(), 5);
    EXPECT_THAT(record.Extension.size(), 23);
    EXPECT_THAT(record.Extension["shared"], Eq("willbeoverwritten"));
    EXPECT_THAT(record.Extension["empty"], Eq(""));
    EXPECT_THAT(record.Extension["parentonly"], Eq("willberemoved"));
    EXPECT_THAT(record.PIIExtensions["sharedpii"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record.PIIExtensions["sharedpii"].Kind, ::AriaProtocol::PIIKind::DistinguishedName);
    EXPECT_THAT(record.PIIExtensions["sharedpii"].RawContent, Eq("willbeoverwrittenpii"));
    EXPECT_THAT(record.PIIExtensions["emptypii"].RawContent, Eq(""));
    EXPECT_THAT(record.PIIExtensions["parentonlypii"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record.PIIExtensions["parentonlypii"].Kind, ::AriaProtocol::PIIKind::GenericData);
    EXPECT_THAT(record.PIIExtensions["parentonlypii"].RawContent, Eq("willberemoved"));


    loggerCtx.setCustomField("shared", "latest");
    loggerCtx.setCustomField("parentonly", "");
	EventProperty prop10("latestpii", PiiKind_MailSubject);
	loggerCtx.setCustomField("sharedpii", prop10);
	EventProperty prop11("", PiiKind_IPv4Address);
    loggerCtx.setCustomField("parentonlypii", prop11);	
    loggerCtx.setCustomField("child", "specific");
	EventProperty prop12("specificpii", PiiKind_QueryString);
    loggerCtx.setCustomField("childpii", prop12 );

    ::AriaProtocol::Record record1;
    loggerCtx.writeToRecord(record1);
    EXPECT_THAT(record1.PIIExtensions.size(), 6);
    EXPECT_THAT(record1.Extension.size(), 24);

    EXPECT_THAT(record1.PIIExtensions["UserInfo.Id"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record1.PIIExtensions["UserInfo.Id"].Kind, ::AriaProtocol::PIIKind::Identity);
    EXPECT_THAT(record1.PIIExtensions["UserInfo.Id"].RawContent, Eq("userId"));

    EXPECT_THAT(record1.PIIExtensions["sharedpii"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record1.PIIExtensions["sharedpii"].Kind, ::AriaProtocol::PIIKind::MailSubject);
    EXPECT_THAT(record1.PIIExtensions["sharedpii"].RawContent, Eq("latestpii"));
    EXPECT_THAT(record1.PIIExtensions["parentpii"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record1.PIIExtensions["parentpii"].Kind, ::AriaProtocol::PIIKind::GenericData);
    EXPECT_THAT(record1.PIIExtensions["parentpii"].RawContent, Eq("willremainpii"));
    EXPECT_THAT(record1.PIIExtensions["childpii"].ScrubType, ::AriaProtocol::PIIScrubber::O365);
    EXPECT_THAT(record1.PIIExtensions["childpii"].Kind, ::AriaProtocol::PIIKind::QueryString);
    EXPECT_THAT(record1.PIIExtensions["childpii"].RawContent, Eq("specificpii"));

    EXPECT_THAT(record1.Extension["shared"], Eq("latest"));
    EXPECT_THAT(record1.Extension["parent"], Eq("willremain"));
    EXPECT_THAT(record1.Extension["child"], Eq("specific"));

    EXPECT_THAT(record1.Extension["AppInfo.Id"], Eq("appId"));
    EXPECT_THAT(record1.Extension["AppInfo.ExperimentIds"], Eq("appExperimentIds"));
    EXPECT_THAT(record1.Extension["AppInfo.Version"], Eq("appVersion"));
    EXPECT_THAT(record1.Extension["AppInfo.Language"], Eq("appLanguage"));

    EXPECT_THAT(record1.Extension["DeviceInfo.Id"], Eq("deviceId"));
    EXPECT_THAT(record1.Extension["DeviceInfo.Make"], Eq("deviceMake"));
    EXPECT_THAT(record1.Extension["DeviceInfo.Model"], Eq("deviceModel"));

    EXPECT_THAT(record1.Extension["DeviceInfo.NetworkCost"], Eq("Unmetered"));
    EXPECT_THAT(record1.Extension["DeviceInfo.NetworkProvider"], Eq("networkProvider"));
    EXPECT_THAT(record1.Extension["DeviceInfo.NetworkType"], Eq("Wired"));

    EXPECT_THAT(record1.Extension["DeviceInfo.OsName"], Eq("osName"));
    EXPECT_THAT(record1.Extension["DeviceInfo.OsVersion"], Eq("osVersion"));
    EXPECT_THAT(record1.Extension["DeviceInfo.OsBuild"], Eq("osBuild"));

    EXPECT_THAT(record1.Extension["UserInfo.MsaId"], Eq("userMsaId"));
    EXPECT_THAT(record1.Extension["UserInfo.ANID"], Eq("userANID"));
    EXPECT_THAT(record1.Extension["UserInfo.AdvertisingId"], Eq("userAdvertingId"));
    EXPECT_THAT(record1.Extension["UserInfo.Language"], Eq("language"));
    EXPECT_THAT(record1.Extension["UserInfo.TimeZone"], Eq("timeZone"));
}

TEST(ContextFieldsProviderTests, UsesPalValues)
{
    ContextFieldsProvider ctx(nullptr);

    ::AriaProtocol::Record record;
    ctx.writeToRecord(record);

    EXPECT_THAT(record.Extension["DeviceInfo.Id"],          Not(IsEmpty()));
    EXPECT_THAT(record.Extension["DeviceInfo.Model"],       Not(IsEmpty()));
    EXPECT_THAT(record.Extension["DeviceInfo.NetworkType"], Not(IsEmpty()));
    EXPECT_THAT(record.Extension["DeviceInfo.OsName"],      Not(IsEmpty()));
    EXPECT_THAT(record.Extension["DeviceInfo.OsVersion"],   Not(IsEmpty()));
}

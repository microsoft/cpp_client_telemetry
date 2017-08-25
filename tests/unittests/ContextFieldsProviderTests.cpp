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

    ::AriaProtocol::CsEvent record;
    loggerCtx.writeToRecord(record);

    EXPECT_THAT(record.data[0].properties.size(), 8);
    EXPECT_THAT(record.data[0].properties["shared"].stringValue, Eq("willbeoverwritten"));
    EXPECT_THAT(record.data[0].properties["empty"].stringValue, Eq(""));
    EXPECT_THAT(record.data[0].properties["parentonly"].stringValue, Eq("willberemoved"));
    EXPECT_THAT(record.data[0].properties["sharedpii"].attributes[0].pii[0].Kind, ::AriaProtocol::PIIKind::DistinguishedName);
    EXPECT_THAT(record.data[0].properties["sharedpii"].stringValue, Eq("willbeoverwrittenpii"));
    EXPECT_THAT(record.data[0].properties["emptypii"].stringValue, Eq(""));
    EXPECT_THAT(record.data[0].properties["parentonlypii"].attributes[0].pii[0].Kind, ::AriaProtocol::PIIKind::GenericData);
    EXPECT_THAT(record.data[0].properties["parentonlypii"].stringValue, Eq("willberemoved"));


    loggerCtx.setCustomField("shared", "latest");
    loggerCtx.setCustomField("parentonly", "");
	EventProperty prop10("latestpii", PiiKind_MailSubject);
	loggerCtx.setCustomField("sharedpii", prop10);
	EventProperty prop11("", PiiKind_IPv4Address);
    loggerCtx.setCustomField("parentonlypii", prop11);	
    loggerCtx.setCustomField("child", "specific");
	EventProperty prop12("specificpii", PiiKind_QueryString);
    loggerCtx.setCustomField("childpii", prop12 );

    ::AriaProtocol::CsEvent record1;
    loggerCtx.writeToRecord(record1);
    EXPECT_THAT(record1.data[0].properties.size(), 10);

    EXPECT_THAT(record1.extUser[0].id, Eq("userId"));

    EXPECT_THAT(record1.data[0].properties["sharedpii"].attributes[0].pii[0].Kind, ::AriaProtocol::PIIKind::MailSubject);
    EXPECT_THAT(record1.data[0].properties["sharedpii"].stringValue, Eq("latestpii"));
    EXPECT_THAT(record1.data[0].properties["parentpii"].attributes[0].pii[0].Kind, ::AriaProtocol::PIIKind::GenericData);
    EXPECT_THAT(record1.data[0].properties["parentpii"].stringValue, Eq("willremainpii"));
    EXPECT_THAT(record1.data[0].properties["childpii"].attributes[0].pii[0].Kind, ::AriaProtocol::PIIKind::QueryString);
    EXPECT_THAT(record1.data[0].properties["childpii"].stringValue, Eq("specificpii"));

    EXPECT_THAT(record1.data[0].properties["shared"].stringValue, Eq("latest"));
    EXPECT_THAT(record1.data[0].properties["parent"].stringValue, Eq("willremain"));
    EXPECT_THAT(record1.data[0].properties["child"].stringValue, Eq("specific"));

    EXPECT_THAT(record1.appId, Eq("appId"));
    EXPECT_THAT(record1.expApp[0].expId, Eq("appExperimentIds"));
    //EXPECT_THAT(record1.data[0].properties["AppInfo.Version"].stringValue, Eq("appVersion"));
    //EXPECT_THAT(record1.data[0].properties["AppInfo.Language"].stringValue, Eq("appLanguage"));

    EXPECT_THAT(record1.extDevice[0].id, Eq("deviceId"));
    //EXPECT_THAT(record1.extDevice[0]..properties["DeviceInfo.Make"].stringValue, Eq("deviceMake"));
    //EXPECT_THAT(record1.data[0].properties["DeviceInfo.Model"].stringValue, Eq("deviceModel"));

   // EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkCost"].stringValue, Eq("Unmetered"));
  //  EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkProvider"].stringValue, Eq("networkProvider"));
  //  EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkType"].stringValue, Eq("Wired"));

    EXPECT_THAT(record1.os, Eq("osName"));
    EXPECT_THAT(record1.osVer, Eq("osVersion"));
    //EXPECT_THAT(record1.data[0].properties["DeviceInfo.OsBuild"].stringValue, Eq("osBuild"));

    //EXPECT_THAT(record1.data[0].properties["UserInfo.MsaId"].stringValue, Eq("userMsaId"));
    //EXPECT_THAT(record1.data[0].properties["UserInfo.ANID"].stringValue, Eq("userANID"));
    //EXPECT_THAT(record1.data[0].properties["UserInfo.AdvertisingId"].stringValue, Eq("userAdvertingId"));
    //EXPECT_THAT(record1.extUser[0].localId, Eq("language"));
    //EXPECT_THAT(record1.extUser[0], Eq("timeZone"));
}

TEST(ContextFieldsProviderTests, UsesPalValues)
{
    ContextFieldsProvider ctx(nullptr);

    ::AriaProtocol::CsEvent record;
    ctx.writeToRecord(record);

    EXPECT_THAT(record.extDevice[0].id,          Not(IsEmpty()));
    EXPECT_THAT(record.extDevice[0].authSecId,   Not(IsEmpty()));
    //EXPECT_THAT(record.data[0].properties["DeviceInfo.NetworkType"].stringValue, Not(IsEmpty()));
    EXPECT_THAT(record.os,      Not(IsEmpty()));
    EXPECT_THAT(record.osVer,   Not(IsEmpty()));
}

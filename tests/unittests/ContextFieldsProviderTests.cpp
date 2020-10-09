//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "api/ContextFieldsProvider.hpp"

using namespace testing;
using namespace MAT;

TEST(ContextFieldsProviderTests, SetProperties)
{
    ContextFieldsProvider ctx(nullptr);
    ContextFieldsProvider loggerCtx(&ctx);

    ctx.SetCustomField("shared", "willbeoverwritten");
    ctx.SetCustomField("parent", "willremain");
    ctx.SetCustomField("empty", "");
    ctx.SetCustomField("parentonly", "willberemoved");
    EventProperty prop("willbeoverwrittenpii", PiiKind_DistinguishedName);
    ctx.SetCustomField("sharedpii", prop);
    EventProperty prop1("willremainpii", PiiKind_GenericData);
    ctx.SetCustomField("parentpii", prop1);
    EventProperty prop2("", PiiKind_Identity);
    ctx.SetCustomField("emptypii", prop2);
    EventProperty prop3("willberemoved", PiiKind_GenericData);
    ctx.SetCustomField("parentonlypii", prop3);

    ctx.SetAppId("appId");
    ctx.SetAppExperimentIds("appExperimentIds");
    ctx.SetAppLanguage("appLanguage");
    ctx.SetAppVersion("appVersion");

    ctx.SetDeviceId("deviceId");
    ctx.SetDeviceMake("deviceMake");
    ctx.SetDeviceModel("deviceModel");
    ctx.SetDeviceOrgId("deviceOrgId");

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

    ::CsProtocol::Record record;
    loggerCtx.writeToRecord(record);

    EXPECT_THAT(record.data[0].properties.size(), 8);
    EXPECT_THAT(record.data[0].properties["shared"].stringValue, Eq("willbeoverwritten"));
    EXPECT_THAT(record.data[0].properties["empty"].stringValue, Eq(""));
    EXPECT_THAT(record.data[0].properties["parentonly"].stringValue, Eq("willberemoved"));
    EXPECT_THAT(record.data[0].properties["sharedpii"].attributes[0].pii[0].Kind, ::CsProtocol::PIIKind::DistinguishedName);
    EXPECT_THAT(record.data[0].properties["sharedpii"].stringValue, Eq("willbeoverwrittenpii"));
    EXPECT_THAT(record.data[0].properties["emptypii"].stringValue, Eq(""));
    EXPECT_THAT(record.data[0].properties["parentonlypii"].attributes[0].pii[0].Kind, ::CsProtocol::PIIKind::GenericData);
    EXPECT_THAT(record.data[0].properties["parentonlypii"].stringValue, Eq("willberemoved"));


    loggerCtx.SetCustomField("shared", "latest");
    loggerCtx.SetCustomField("parentonly", "");
    EventProperty prop10("latestpii", PiiKind_MailSubject);
    loggerCtx.SetCustomField("sharedpii", prop10);
    EventProperty prop11("", PiiKind_IPv4Address);
    loggerCtx.SetCustomField("parentonlypii", prop11);
    loggerCtx.SetCustomField("child", "specific");
    EventProperty prop12("specificpii", PiiKind_QueryString);
    loggerCtx.SetCustomField("childpii", prop12);

    ::CsProtocol::Record record1;
    loggerCtx.writeToRecord(record1);
    EXPECT_THAT(record1.data[0].properties.size(), 10);

    EXPECT_THAT(record1.extUser[0].localId, Eq("userId"));

    EXPECT_THAT(record1.data[0].properties["sharedpii"].attributes[0].pii[0].Kind, ::CsProtocol::PIIKind::MailSubject);
    EXPECT_THAT(record1.data[0].properties["sharedpii"].stringValue, Eq("latestpii"));
    EXPECT_THAT(record1.data[0].properties["parentpii"].attributes[0].pii[0].Kind, ::CsProtocol::PIIKind::GenericData);
    EXPECT_THAT(record1.data[0].properties["parentpii"].stringValue, Eq("willremainpii"));
    EXPECT_THAT(record1.data[0].properties["childpii"].attributes[0].pii[0].Kind, ::CsProtocol::PIIKind::QueryString);
    EXPECT_THAT(record1.data[0].properties["childpii"].stringValue, Eq("specificpii"));

    EXPECT_THAT(record1.data[0].properties["shared"].stringValue, Eq("latest"));
    EXPECT_THAT(record1.data[0].properties["parent"].stringValue, Eq("willremain"));
    EXPECT_THAT(record1.data[0].properties["child"].stringValue, Eq("specific"));

    EXPECT_THAT(record1.extApp[0].id, Eq("appId"));
    EXPECT_THAT(record1.extApp[0].expId, Eq("appExperimentIds"));
    //EXPECT_THAT(record1.data[0].properties["AppInfo.Version"].stringValue, Eq("appVersion"));
    //EXPECT_THAT(record1.data[0].properties["AppInfo.Language"].stringValue, Eq("appLanguage"));

    EXPECT_THAT(record1.extDevice[0].localId, Eq("c:deviceId"));
    EXPECT_THAT(record1.extDevice[0].orgId, Eq("deviceOrgId"));

    //EXPECT_THAT(record1.extDevice[0]..properties["DeviceInfo.Make"].stringValue, Eq("deviceMake"));
    //EXPECT_THAT(record1.data[0].properties["DeviceInfo.Model"].stringValue, Eq("deviceModel"));

   // EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkCost"].stringValue, Eq("Unmetered"));
  //  EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkProvider"].stringValue, Eq("networkProvider"));
  //  EXPECT_THAT(record1.data[0].properties["DeviceInfo.NetworkType"].stringValue, Eq("Wired"));

    EXPECT_THAT(record1.extOs[0].name, Eq("osName"));
    EXPECT_THAT(record1.extOs[0].ver, Eq("osBuild"));
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

    ::CsProtocol::Record record;
    ctx.writeToRecord(record);

    EXPECT_THAT(record.extDevice[0].localId, Not(IsEmpty()));
    //EXPECT_THAT(record.extDevice[0].authSecId,   Not(IsEmpty()));
    //EXPECT_THAT(record.data[0].properties["DeviceInfo.NetworkType"].stringValue, Not(IsEmpty()));
    EXPECT_THAT(record.extOs[0].name, Not(IsEmpty()));
    EXPECT_THAT(record.extOs[0].ver, Not(IsEmpty()));
}

class TestContextFieldsProvider : public ContextFieldsProvider
{
public:
	std::map<std::string, std::string>& GetCommonContextEventToConfigIds() noexcept
	{
		return m_commonContextEventToConfigIds;
	}

};

TEST(ContextFieldsProviderTests, EventExperimentIdsStartEmpty)
{
	TestContextFieldsProvider provider;
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds().size(), 0);
}

TEST(ContextFieldsProviderTests, SetEventExperimentIds_EmptyEventNameDoesntChangeContext)
{
	TestContextFieldsProvider provider;
	provider.SetEventExperimentIds(std::string {}, "Fred");
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds().size(), 0);
}

TEST(ContextFieldsProviderTests, SetEventExperimentIds_MixedUpperAndLoerCaseEventNameNormalizedToLowerCase)
{
	TestContextFieldsProvider provider;
	provider.SetEventExperimentIds("Rodgers", "Fred");
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds().size(), 1);
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds()["rodgers"], Eq("Fred"));
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds().find("Rodgers"), provider.GetCommonContextEventToConfigIds().end());
}

TEST(ContextFieldsProviderTests, SetEventExperimentIds_SameKeyUpdatesValue)
{
	TestContextFieldsProvider provider;
	provider.SetEventExperimentIds("Rodgers", "Fred");
	provider.SetEventExperimentIds("rodgers", "Mister");
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds()["rodgers"], Eq("Mister"));
}

TEST(ContextFieldsProviderTests, SetEventExperimentIds_EmptyExperimentIdsRemovesEntry)
{
	TestContextFieldsProvider provider;
	provider.SetEventExperimentIds("Rodgers", "Fred");
	provider.SetEventExperimentIds("Rodgers", "");
	EXPECT_THAT(provider.GetCommonContextEventToConfigIds().size(), 0);
}

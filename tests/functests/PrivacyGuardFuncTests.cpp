// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"

#define HAVE_MAT_DEFAULTDATAVIEWER

#if defined __has_include
#if __has_include("modules/PrivacyGuard/PrivacyGuard.hpp")
#include "modules/PrivacyGuard/PrivacyGuard.hpp"
#else
/* Compiling without Data Viewer */
#undef HAVE_MAT_DEFAULTDATAVIEWER
#endif
#endif

#ifdef HAVE_MAT_DEFAULTDATAVIEWER

#include "NullObjects.hpp"
#include <IDataInspector.hpp>
#include <LogManager.hpp>

#include <algorithm>
#include <functional>

using namespace testing;
using namespace MAT;

class MockLogger : public NullLogger
{
   public:
    std::function<void(const EventProperties& properties)> m_logEventOverride;
    virtual void LogEvent(EventProperties const& properties) override
    {
        if (m_logEventOverride)
        {
            m_logEventOverride(properties);
        }
    }
};

static const std::string c_testEventName{"Office.TestEvent"};
static const std::string c_testFieldName{"Data.TheField"};
static const std::string c_testTargetTenant{"0ab12345cd6e78f9012g3456hi7jk890-l1m2345n-6789-0op1-qr23-st4567u8vwx9-0123"};
static const std::string c_testComputerName{"Motherboard"};
static const std::string c_testDomain{"TEST.CONTOSO.COM"};
static const std::string c_testUserName{"Awesome Username"};
static const std::string c_testUserAlias{"awesomeuser"};
static const std::string c_testClientId{"43efb3b1-c7a3-4f29-beea-63ccb28160ac"};
static const std::string c_testSusClientId{"e1b2ece8-2451-4ea9-997a-6f37b50be8de"};
static const std::string c_testSqmId{"7d06a83a-200d-4ccb-bfc6-d0995c840bde"};
static const std::string c_testC2rInstallId{"0450fe66-aeed-4059-99ca-4dd8702cbd1f"};
static const std::string c_testLanguageId{"en-US"};
static const std::string c_testLanguageName{"English (United States)"};
static const std::string c_testIPv4{"192.168.1.1"};
static const std::string c_testIPv6{"1234:4578:9abc:def0:bea4:ca4:ca1:d0g"};
static const std::string c_testNumbers{"12345.6"};
static const std::string c_testEmail{"test.some@email.com"};
static const std::string c_testAdalGuid{"197648AE-E0E1-4115-962E-29C97E5CD101_ADAL"};
static const GUID_t c_testGuid = {0x197648ae, 0xe0e1, 0x4115, {0x96, 0x2e, 0x29, 0xc9, 0x7e, 0x5c, 0xd1, 0x1}};
#define TEST_TOKEN "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

class PrivacyGuardFuncTests : public ::testing::Test
{
   public:
    static CommonDataContexts GenerateTestDataContexts() noexcept
    {
        CommonDataContexts cdc;
        cdc.UserName = c_testUserName;
        cdc.UserAlias = c_testUserAlias;
        cdc.DomainName = c_testDomain;
        cdc.MachineName = c_testComputerName;
        cdc.IpAddresses.push_back("10.0.1.1");
        cdc.IpAddresses.push_back(c_testIPv4);
        cdc.IpAddresses.push_back(c_testIPv6);
        cdc.LanguageIdentifiers.push_back(c_testLanguageId);
        cdc.LanguageIdentifiers.push_back(c_testLanguageName);
        cdc.MachineIds.push_back(c_testC2rInstallId);
        cdc.OutOfScopeIdentifiers.push_back(c_testClientId);
        cdc.OutOfScopeIdentifiers.push_back(c_testSqmId);
        cdc.OutOfScopeIdentifiers.push_back(c_testSusClientId);
        return cdc;
    }

    static std::string MakeNonIntuitiveString()
    {
        // There are some functions for fetching Machine names that can generate
        // weird string such as this. Testing for those as well.
        std::string output;
        output.resize(10, 'a');
        output[0] = '\0';

        return output;
    }

    static std::string GenerateIdentifierVariant(const std::string& input, bool uppercase, bool removeDashes)
    {
        std::string value;

        if (input.length() >= m_maxValueLength)
            return input;
        if (uppercase)
        {
            value = toUpper(input);
        }
        else
        {
            value = input;
        }

        if (removeDashes)
        {
            value.erase(std::remove(value.begin(), value.end(), '-'), value.end());
        }
        return value;
    }

   private:
    static const size_t m_maxValueLength{256};
};

TEST_F(PrivacyGuardFuncTests, InitializePrivacyGuard)
{
    auto mockLogger = new MockLogger();
    LogManager::Initialize();
    ASSERT_NO_THROW([&mockLogger]() {
        ASSERT_FALSE(LogManager::GetInstance()->GetDataInspectorState());
        LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
        ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    });
    ASSERT_NO_THROW([&mockLogger]() {
        ASSERT_FALSE(LogManager::GetInstance()->GetDataInspectorState());
        LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger, nullptr);
        ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    });
    ASSERT_NO_THROW([&mockLogger]() {
        ASSERT_FALSE(LogManager::GetInstance()->GetDataInspectorState());
        LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger, std::move(std::make_unique<CommonDataContexts>()));
        ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    });

    delete mockLogger;
}

TEST_F(PrivacyGuardFuncTests, SetDataInspectorState)
{
    auto mockLogger = new MockLogger();
    LogManager::Initialize();
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    LogManager::GetInstance()->SetDataInspectorState(/*isEnabled:*/ false);
    ASSERT_FALSE(LogManager::GetInstance()->GetDataInspectorState());
    LogManager::GetInstance()->SetDataInspectorState(/*isEnabled:*/ true);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
}

TEST_F(PrivacyGuardFuncTests, OverrideDataInspector)
{
    auto mockLogger = new MockLogger();
    LogManager::Initialize();
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    LogManager::GetInstance()->OverrideDataInspector(nullptr);
    ASSERT_FALSE(LogManager::GetInstance()->GetDataInspectorState());
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());

    delete mockLogger;
}


TEST_F(PrivacyGuardFuncTests, SetCommonDataContexts)
{
    auto mockLogger = new MockLogger();
    LogManager::Initialize();
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    LogManager::GetInstance()->SetCommonDataContextsForInspection(std::move(std::make_unique<CommonDataContexts>()));

    delete mockLogger;
}

TEST_F(PrivacyGuardFuncTests, AddIgnoredConcern)
{
    auto mockLogger = new MockLogger();
    LogManager::Initialize();
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());
    std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>> ignoredConcerns;
    ignoredConcerns.push_back(std::make_tuple(c_testEventName, c_testFieldName, DataConcernType::InScopeIdentifier));
    LogManager::GetInstance()->AddIgnoredConcern(ignoredConcerns);

    delete mockLogger;
}

TEST_F(PrivacyGuardFuncTests, GetAllConcerns_EmailMatching)
{
    auto mockLogger = new MockLogger();
    auto privacyConcernLogCount = 0;
    mockLogger->m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::Initialize(TEST_TOKEN);
    LogManager::GetInstance()->InitializePrivacyGuardDataInspector(mockLogger);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspectorState());

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Some%2eone%40Microsoft%2ecom"); //ConcernType::InternalEmailAddress  //As happens in escaped URLs
    props.SetProperty("Field2", "Someone@Microsoft.com"); //ConcernType::InternalEmailAddress
    props.SetProperty("Field3", "Some.one@Exchange.Microsoft.com"); //ConcernType::InternalEmailAddress
    props.SetProperty("Field4", "Some_one@microsoft_com"); //ConcernType::InternalEmailAddress
    props.SetProperty("Field5", "Some_one_AT_microsoft_com"); //ConcernType::InternalEmailAddress
    props.SetProperty("Field6", "Microsoft.com");
    props.SetProperty("Field7", "Exchange.Microsoft.com");
    props.SetProperty("Field8", "Some_one");
    logger->LogEvent(props);
    ASSERT_EQ(5, privacyConcernLogCount);
    delete mockLogger;
}

//TEST(PrivacyGuardFuncTests, GetAllConcerns_UrlMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.microsoft.com", DataConcernType::Url));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "HTTPS://www.microsoft.com", DataConcernType::Url));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "File://www.microsoft.com", DataConcernType::Url));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com", DataConcernType::Url));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_FileSharingUrlMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.dropbox.com/aaaaa", DataConcernType::FileSharingUrl));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "HTTPS://wopi.dropbox.com/aaaaaa", DataConcernType::FileSharingUrl));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "File://loggingShare/MyLogFolder", DataConcernType::FileSharingUrl));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Having https://wopi.dropbox.com/ appear in the middle of text is expected to confuse the detector even though there's no file encoding", DataConcernType::FileSharingUrl));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "But talking about http and dropbox.com/ should not."));
//
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "https://outlook.live.com/owa/wopi/files/[guid]@outlook.com/[encoded_goo]", DataConcernType::FileSharingUrl));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "https://outlook.live.com/owa/wopi.ashx/files/[guid]@outlook.com/[encoded_goo]", DataConcernType::FileSharingUrl));
//
//    //Not file-sharing if just talking about the domain.
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com/", DataConcernType::FileSharingUrl));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com", DataConcernType::FileSharingUrl));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_SecurityChecks)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.dropbox.com/aaaaa", DataConcernType::FileSharingUrl));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&AWSAccessKeyId=abc", DataConcernType::Security));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&Signature=abc", DataConcernType::Security));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&Access_token=abc", DataConcernType::Security));
//
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "But talking about AWSAccessKey and Signature is not flagged."));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_ContentFormatMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "<HTML><P><Table>", DataConcernType::Content));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "<?xml version=\\\"1.0\\\" encoding=\\\"utf - 8\\\"?><UnifiedRules xmlns=\\\"urn: UnifiedRules\\\" xmlns:xsi=\\\"http://www.w3.org/2001/XMLSchema-instance\\\" xsi:schemaLocation=\\\"urn:UnifiedRules ../../RulesSchema/Schemas/UnifiedRules.xsd\\\">", DataConcernType::Content));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "{\\rtf1\\adeflang1025\\ansi\\ansicpg1252\\uc1\\adeff0\\deff0\\stshfdbch0\\stshfloch39\\stshfhich39\\stshfbi39\\deflang1033\\deflangfe1033\\themelang1033\\themelangfe0\\themelangcs0{\\fonttbl{\\f0\\fbidi \\froman\\fcharset0\\fprq2{\\*\\panose 02020603050405020304}Times New Roman{\\*\\falt Times};}", DataConcernType::Content));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "<asp:TableCell ID=\\\"TableCell10\\\" runat=\\\"server\\\">", DataConcernType::Content));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "MIME-Version:1.0", DataConcernType::Content));
//
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "HTML rtf xml"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PidKeyMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "1A2B3-C4D5E-6F7H8-I9J0K-LMNOP", DataConcernType::PIDKey));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_UserMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "This content brought to you by awesomeuser and theletterdee", DataConcernType::UserAlias));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Email awesomeuser at microsoft for more info. Don't really, this is just a test", DataConcernType::UserAlias));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "AWESOMEUSER should be flagged.", DataConcernType::UserAlias));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "awesomeuser should be flagged too.", DataConcernType::UserAlias));
//
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Not expected to find awesome user when case is different to avoid false positives from 'names' in words"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "awesomeusers should not be flagged because the matched alias is part of a longer word"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "lawesomeuser should not be flagged because the matched alias is part of a longer word"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PrettyUserMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "This content brought to you by Awesome Username and theletterdee", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Email Awesome Username at microsoft for more info. Don't really, this is just a test", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome brought free cake today!", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Turns out the cake is a lie. Don't believe Username.", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, c_testUserName, DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Feature=Awesome", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome=Feature", DataConcernType::UserName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Username", DataConcernType::UserName));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "AwesomeFeature", DataConcernType::UserName));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "FeatureAwesome", DataConcernType::UserName));
//
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Many names are commonly seen in word substrings. Like BUDdy, genERIC, etc. awesome username and AWESOME USERNAME shouldn't get flagged."));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PrettyUserMatching_NumbersDoNotCount)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForNumericTest(std::make_shared<MockLogger>());
//    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testNumbers, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//
//    issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"12345.6 stuff"}, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PrettyUserMatching_NonIsolatedWordsNotMatched)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(
//        std::make_shared<MockLogger>(),
//        "Office Automation Limited Client",
//        "dozer",
//        c_testDomain,
//        c_testComputerName);
//
//    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"Office has an Automation Client. It's Limited."}, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PrettyUserMatching_ShortNamesNotMached)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(
//        std::make_shared<MockLogger>(),
//        "A Guy",
//        "dozer",
//        c_testDomain,
//        c_testComputerName);
//
//    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"His name is A Guy."}, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//
//    issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"A person named Guy walks down the road."}, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_MachineName)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "MOTHERBOARD should be flagged.", DataConcernType::MachineName));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "motherboard should be flagged too.", DataConcernType::MachineName));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Not expected to find awesome user when case is different to avoid false positives from 'names' in words"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "motherboarding should not be flagged because the matched name is part of a longer word."));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "grandmotherboard should not be flagged because the matched name is part of a longer word."));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_MachineName_NonIntuitiveStrings)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(
//        std::make_shared<MockLogger>(),
//        c_testUserName,
//        PrivacyGuardFuncTests::MakeNonIntuitiveString(),
//        c_testDomain,
//        PrivacyGuardFuncTests::MakeNonIntuitiveString());
//
//    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"Nothing to find here."}, c_testTargetTenant);
//    ASSERT_EQ(0, issues.size());
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_DomainName)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "TEST.MICROSOFT.COM should be flagged.", DataConcernType::UserDomain));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "test.microsoft.com should be flagged too.", DataConcernType::UserDomain));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_BannedIdentityTypes)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "*_SSPI"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "*_AD"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_SSPI"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_AD"));
//
//    //SSPI format includes the '@' so will be picked up by the other email-finding code. Not sure about AD.
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email@contoso.com_SSPI", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email@microsoft.com_SSPI", DataConcernType::InternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email@contoso.com_AD", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email@microsoft.com_AD", DataConcernType::InternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email_contoso.com_SSPI", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email_microsoft.com_SSPI", DataConcernType::InternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email_contoso.com_AD", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "email_microsoft.com_AD", DataConcernType::InternalEmailAddress));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_DirectoryMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "C:\\Users\\sunshine\\AppData\\Loca", DataConcernType::Directory));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "C:\\\\Users\\\\sunshine\\\\AppData\\\\Loca", DataConcernType::Directory));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "\\\\Users\\sunshine\\AppData\\Loca", DataConcernType::Directory));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Failure to rain on c:\\\\Users\\\\sunshine", DataConcernType::Directory));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "{ \"JobID\" : \"341729903\",\"ScenarioPath\" : \"OfficeVSO\\\\OC\\\\Word\\\\Core\\\\File IO and Collaboration\\\\RTT\\\\Track Changes\"}", DataConcernType::Directory));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "{UnicodeEscapeFalsePositive:\\u0027formulaODataFeeds\\u0027}"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Diagnostic Context:\\n    Info: 1234\\n"));  //Some warnings in Mac Outlook cause this false positive due to "t:\n".
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Failed to open key HKEY_LOCAL_MACHINE\\\\Software\\\\Microsoft\\\\Office\\\\16.0\\\\Common\\\\Feature"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_ExternalEmailMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Using the seeing stones will send an email alert to Sauron@contoso.com", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "and also cc Saruman@contoso.com", DataConcernType::ExternalEmailAddress));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_LocationMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Location: Moonbuggy", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Latitude: 6deg North", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Longitude: 12deg West", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Coordinates: 5 by 5", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Coord: Castle 5", DataConcernType::Location));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Coordinator: Castle 5"));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "GeoLocation: Pit of despair", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Geo: At the end of the rainbow", DataConcernType::Location));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "GeoID: Shelob's bed & breakfast", DataConcernType::Location));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Geometric: Shelob's bed & breakfast"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_FilesMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.foo", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "NothingToSeeHere.xlsx", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Vacation.JPG", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, ".JPG", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, ".JPG.SomeOtherStuff", DataConcernType::FileNameOrExtension));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "MyAddin.dl"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "TheApp.exe"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common.com"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_PostFixedFilesMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx.CustomCommand", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx.Custom_Command", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_Custom_Command", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_CustomCommand", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_CustomCommand_", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx.BecauseILikeChopsticks", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx.BecauseILikeChopsticks.", DataConcernType::FileNameOrExtension));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "My_Special.Oddly.Formatted.pptx_item", DataConcernType::FileNameOrExtension));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "My_Special.Oddly.Formatted.free_item"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common."));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, ".Common"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "...Common"));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common..."));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common.________.Uncommon"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_InScopeIdentifiers)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_ADAL", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_1::0123456789ABCDEF_ADAL", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_1:live.com:0123456789ABCDEF_ADAL", DataConcernType::InScopeIdentifier));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_ADAL", DataConcernType::ExternalEmailAddress));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90AB-CDEF-FEDC-BA9876543210", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef_OrgId", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef_LiveId", DataConcernType::InScopeIdentifier));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef", DataConcernType::InScopeIdentifier));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "adal, liveid, orgid, sspi"));
//
//    const char* testGuidsz = "{197648AE-E0E1-4115-962E-29C97E5CD101}";
//
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "197648AE-E0E1-4115-962E-29C97E5CD101_ADAL", DataConcernType::InScopeIdentifier));
//    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testGuid, c_testTargetTenant);
//    auto issueMatch = std::find_if(issues.cbegin(), issues.cend(), [&](const PrivacyConcern& x) {
//        return x.DataConcernType == DataConcernType::InScopeIdentifier && x.EventName == c_testEventName && x.FieldName == c_testFieldName && x.FieldValue == testGuidsz;
//    });
//
//    ASSERT_EQ(issues.cend(), issueMatch);
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_DemographicsMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "I English good speak", DataConcernType::DemographicInfoLanguage));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "The app is set to EN-US", DataConcernType::DemographicInfoLanguage));
//    ASSERT_TRUE(PrivacyGuardFuncTests::IsExpectedDataConcern(testPrivacyGuard, "Made in the United States", DataConcernType::DemographicInfoCountryRegion));
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "The ancient tablet States: United the lost 8 pieces of sandwitch to achieve ultimate lunch!"));
//}
//
//TEST(PrivacyGuardFuncTests, GetAllConcerns_OutOfScopeIdentifiersMatching)
//{
//    const auto& testPrivacyGuard = PrivacyGuardFuncTests::GetPrivacyGuardForTest(std::make_shared<MockLogger>());
//    PrivacyGuardFuncTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testClientId);
//    PrivacyGuardFuncTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testSqmId);
//    PrivacyGuardFuncTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testSusClientId);
//
//    ASSERT_FALSE(PrivacyGuardFuncTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "cbfd6749-165c-41c8-a85e-b9c8b8c1f9ce"));
//}
//
//TEST(PrivacyGuardFuncTests, InspectSemanticContext_CheckStringValue_NotifiesIssueCorrectly)
//{
//    auto mockLogger = std::make_shared<MockLogger>();
//    auto logEventCalled = false;
//    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
//        ASSERT_EQ(properties.GetName(), "PrivacyConcern");
//        auto props = properties.GetProperties();
//        auto type = props.find("TypeAsText");
//        ASSERT_NE(type, props.end());
//        ASSERT_EQ(type->second.as_string, PrivacyGuard::DataConcernTypeAsText(DataConcernType::ExternalEmailAddress));
//        logEventCalled = true;
//    };
//    PrivacyGuard pgInstance(mockLogger, nullptr);
//    pgInstance.InspectSemanticContext(c_testFieldName, c_testEmail, true, c_testTargetTenant);
//    ASSERT_TRUE(logEventCalled);
//}
//
//TEST(PrivacyGuardFuncTests, InspectSemanticContext_CheckGuidValue_NotifiesIssueCorrectly)
//{
//    auto mockLogger = std::make_shared<MockLogger>();
//    auto logEventCalled = false;
//    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
//        ASSERT_EQ(properties.GetName(), "PrivacyConcern");
//        auto props = properties.GetProperties();
//        auto type = props.find("TypeAsText");
//        ASSERT_NE(type, props.end());
//        ASSERT_EQ(type->second.as_string, PrivacyGuard::DataConcernTypeAsText(DataConcernType::InScopeIdentifier));
//        logEventCalled = true;
//    };
//    PrivacyGuard pgInstance(mockLogger, nullptr);
//    pgInstance.InspectSemanticContext(c_testFieldName, c_testAdalGuid, true, c_testTargetTenant);
//    ASSERT_TRUE(logEventCalled);
//    logEventCalled = false;
//    pgInstance.InspectSemanticContext(c_testFieldName, c_testGuid, true, c_testTargetTenant);
//    ASSERT_TRUE(logEventCalled);
//}
//
//TEST(PrivacyGuardFuncTests, InspectSemanticContext_IgnoredConcern_DoesNotNotifyIssue)
//{
//    auto mockLogger = std::make_shared<MockLogger>();
//    auto logEventCalled = false;
//    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& /*properties*/) noexcept {
//        logEventCalled = true;
//    };
//    TestPrivacyGuard pgInstance(mockLogger, nullptr);
//
//    std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>> ignoredConcern;
//    ignoredConcern.push_back(std::make_tuple(c_testEventName, c_testFieldName, DataConcernType::InScopeIdentifier));
//
//    pgInstance.AddIgnoredConcern(ignoredConcern);
//    auto results = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testAdalGuid, c_testTargetTenant);
//    ASSERT_FALSE(logEventCalled);
//    logEventCalled = false;
//    results = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testAdalGuid, c_testTargetTenant);
//    pgInstance.InspectSemanticContext(c_testFieldName, c_testGuid, true, c_testTargetTenant);
//    ASSERT_FALSE(logEventCalled);
//}

#else

#endif

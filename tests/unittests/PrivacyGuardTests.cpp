// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"

#define HAVE_MAT_DEFAULTDATAVIEWER

#if defined __has_include
#if __has_include("modules/PrivacyGuard/PrivacyGuard.hpp")
#include "modules/PrivacyGuard/FileTypes.hpp"
#include "modules/PrivacyGuard/PrivacyGuard.hpp"
#else
/* Compiling without Data Viewer */
#undef HAVE_MAT_DEFAULTDATAVIEWER
#endif
#endif

#ifdef HAVE_MAT_DEFAULTDATAVIEWER

#include "CheckForExceptionOrAbort.hpp"
#include "ILogger.hpp"
#include "NullObjects.hpp"

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

class TestPrivacyGuard : public PrivacyGuard
{
   public:
    TestPrivacyGuard(ILogger* loggerInstance, std::unique_ptr<CommonDataContexts>&& commonContexts) :
        PrivacyGuard(loggerInstance, std::move(commonContexts))
    {
    }

    virtual ~TestPrivacyGuard() = default;

    using PrivacyGuard::AddCustomGuidValueInspector;
    using PrivacyGuard::AddCustomStringValueInspector;
    using PrivacyGuard::GetAllPrivacyConcerns;
};

static const std::string c_testEventName{"Office.TestEvent"};
static const std::string c_testFieldName{"Data.TheField"};
static const std::string c_testTargetTenant{"0ab12345cd6e78f9012g3456hi7jk890-l1m2345n-6789-0op1-qr23-st4567u8vwx9-0123"};
static const std::string c_testComputerName{"Motherboard"};
static const std::string c_testDomain{"TEST.MICROSOFT.COM"};
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

class PrivacyGuardTests : public ::testing::Test
{
   public:
    static std::shared_ptr<TestPrivacyGuard> GetPrivacyGuardForTest(ILogger* testLogger)
    {
        return std::make_shared<TestPrivacyGuard>(testLogger, std::move(std::make_unique<CommonDataContexts>(GenerateTestDataContexts())));
    }

    static std::shared_ptr<TestPrivacyGuard> GetPrivacyGuardForTest(
        ILogger* testLogger,
        const std::string& userName,
        const std::string& userAlias,
        const std::string& domainName,
        const std::string& machineName)
    {
        auto cdc = std::make_unique<CommonDataContexts>(GenerateTestDataContexts());
        cdc->UserName = userName;
        cdc->UserAlias = userAlias;
        cdc->DomainName = domainName;
        cdc->MachineName = machineName;

        return std::make_shared<TestPrivacyGuard>(testLogger, std::move(cdc));
    }

    static std::shared_ptr<TestPrivacyGuard> GetPrivacyGuardForNumericTest(ILogger* testLogger)
    {
        auto cdc = std::make_unique<CommonDataContexts>(GenerateTestDataContexts());
        cdc->UserName = "";
        cdc->UserAlias = c_testNumbers;
        cdc->DomainName = c_testNumbers;
        cdc->MachineName = c_testNumbers;

        return std::make_shared<TestPrivacyGuard>(testLogger, std::move(cdc));
    }

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

    static bool IsExpectedDataConcern(const std::shared_ptr<TestPrivacyGuard>& privacyGuardTestInstance, const std::string& value, DataConcernType type)
    {
        auto issues = privacyGuardTestInstance->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, value, c_testTargetTenant);  //(c_testEventName, c_testFieldName, value, c_testTargetTenant);

        return std::find_if(issues.cbegin(), issues.cend(), [&](const PrivacyConcern& x) {
                   return x.DataConcernType == type && x.EventName == c_testEventName && x.FieldName == c_testFieldName && x.FieldValue == value;
               }) != issues.cend();
    }

    static bool IdentifiedAnyDataConcerns(const std::shared_ptr<TestPrivacyGuard>& privacyGuardTestInstance, const std::string& value)
    {
        auto issues = privacyGuardTestInstance->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, value, c_testTargetTenant);
        return issues.size() > 0;
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

    static ::CsProtocol::Record GenerateTestRecord(const std::string& customPartCValue)
    {
        ::CsProtocol::Value testStringValue;
        testStringValue.stringValue = customPartCValue;
        testStringValue.type = ::CsProtocol::ValueKind::ValueString;

        ::CsProtocol::Value testGuidValue;
        uint8_t guidBytes[16];
        c_testGuid.to_bytes(guidBytes);
        std::vector<uint8_t> guidVector;
        guidVector.insert(guidVector.begin(), std::begin(guidBytes), std::end(guidBytes));
        testGuidValue.guidValue.push_back(guidVector);
        testGuidValue.type = ::CsProtocol::ValueKind::ValueGuid;

        ::CsProtocol::Data data;
        data.properties.emplace(std::string{"Data.StringField"}, testStringValue);
        data.properties.emplace(std::string{"Data.GuidField"}, testGuidValue);

        ::CsProtocol::Record testRecord;
        testRecord.data.push_back(data);
        testRecord.name = c_testEventName;
        testRecord.iKey = c_testTargetTenant;

        return testRecord;
    }

    static void ValidateOutOfScopeIdentifierIsFlagged(const std::shared_ptr<TestPrivacyGuard>& privacyGuardTestInstance, const std::string& identifier)
    {
        ASSERT_TRUE(identifier.length() < m_maxValueLength);
        ASSERT_TRUE(IsExpectedDataConcern(privacyGuardTestInstance, identifier, DataConcernType::OutOfScopeIdentifier));

        //Should find case insensitive and with or without dashes in GUIDs
        ASSERT_TRUE(IsExpectedDataConcern(privacyGuardTestInstance, GenerateIdentifierVariant(identifier, true, false), DataConcernType::OutOfScopeIdentifier));
        ASSERT_TRUE(IsExpectedDataConcern(privacyGuardTestInstance, GenerateIdentifierVariant(identifier, false, true), DataConcernType::OutOfScopeIdentifier));
        ASSERT_TRUE(IsExpectedDataConcern(privacyGuardTestInstance, GenerateIdentifierVariant(identifier, true, true), DataConcernType::OutOfScopeIdentifier));
    }

   private:
    static const size_t m_maxValueLength{256};
};

TEST(PrivacyGuardTests, Constructor_LoggerInstanceNotProvided_LoggerInstanceThrowsInvalidArgument)
{
    CheckForExceptionOrAbort<std::invalid_argument>([]() { PrivacyGuard(nullptr, nullptr); });
}

TEST(PrivacyGuardTests, Constructor_LoggerInstanceProvided_InitializedSuccessfully)
{
    auto mockLogger = new MockLogger();

    PrivacyGuard pg(mockLogger);
    ASSERT_TRUE(pg.GetState());
    ASSERT_FALSE(pg.AreCommonPrivacyContextSet());

    delete mockLogger;
}

TEST(PrivacyGuardTests, Constructor_CommonDataContextsProvided_CommonDataContextsSetSuccessfully)
{
    auto mockLogger = new MockLogger();
    auto commonDataContexts = std::make_unique<CommonDataContexts>(PrivacyGuardTests::GenerateTestDataContexts());

    PrivacyGuard pg(mockLogger, std::move(commonDataContexts));
    ASSERT_TRUE(pg.GetState());
    ASSERT_TRUE(pg.AreCommonPrivacyContextSet());

    delete mockLogger;
}

TEST(PrivacyGuardTests, SetState_SetStateToDisabled_StateUpdatedCorrectly)
{
    auto mockLogger = new MockLogger();
    PrivacyGuard pg(mockLogger);
    ASSERT_TRUE(pg.GetState());
    pg.SetState(false);
    ASSERT_FALSE(pg.GetState());

    delete mockLogger;
}

TEST(PrivacyGuardTests, DelaySetCommonPrivacyContext_CommonDataContextsNotProvided_CommonDataContextsNotChanged)
{
    auto mockLogger = new MockLogger();
    auto commonDataContexts = std::make_unique<CommonDataContexts>(PrivacyGuardTests::GenerateTestDataContexts());

    PrivacyGuard pg(mockLogger, std::move(commonDataContexts));
    ASSERT_TRUE(pg.AreCommonPrivacyContextSet());
    pg.AppendCommonDataContext(nullptr);
    ASSERT_TRUE(pg.AreCommonPrivacyContextSet());

    delete mockLogger;
}

TEST(PrivacyGuardTests, DelaySetCommonPrivacyContext_CommonDataContextsProvided_CommonDataContextsChanged)
{
    auto mockLogger = new MockLogger();
    auto commonDataContexts = std::make_unique<CommonDataContexts>(PrivacyGuardTests::GenerateTestDataContexts());

    PrivacyGuard pg(mockLogger);
    ASSERT_FALSE(pg.AreCommonPrivacyContextSet());
    pg.AppendCommonDataContext(std::move(commonDataContexts));
    ASSERT_TRUE(pg.AreCommonPrivacyContextSet());

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_EmailMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Some%2eone%40Microsoft%2ecom", DataConcernType::InternalEmailAddress));  //As happens in escaped URLs
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Someone@Microsoft.com", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Some.one@Exchange.Microsoft.com", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Some_one@microsoft_com", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Some_one_AT_microsoft_com", DataConcernType::InternalEmailAddress));

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Microsoft.com"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Exchange.Microsoft.com"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Some_one"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_UrlMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.microsoft.com", DataConcernType::Url));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "HTTPS://www.microsoft.com", DataConcernType::Url));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "File://www.microsoft.com", DataConcernType::Url));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com", DataConcernType::Url));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_FileSharingUrlMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.dropbox.com/aaaaa", DataConcernType::FileSharingUrl));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "HTTPS://wopi.dropbox.com/aaaaaa", DataConcernType::FileSharingUrl));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "File://loggingShare/MyLogFolder", DataConcernType::FileSharingUrl));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Having https://wopi.dropbox.com/ appear in the middle of text is expected to confuse the detector even though there's no file encoding", DataConcernType::FileSharingUrl));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "But talking about http and dropbox.com/ should not."));

    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "https://outlook.live.com/owa/wopi/files/[guid]@outlook.com/[encoded_goo]", DataConcernType::FileSharingUrl));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "https://outlook.live.com/owa/wopi.ashx/files/[guid]@outlook.com/[encoded_goo]", DataConcernType::FileSharingUrl));

    //Not file-sharing if just talking about the domain.
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com/", DataConcernType::FileSharingUrl));
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Download failed for domain https://wopi.dropbox.com", DataConcernType::FileSharingUrl));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_SecurityChecks)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.dropbox.com/aaaaa", DataConcernType::FileSharingUrl));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&AWSAccessKeyId=abc", DataConcernType::Security));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&Signature=abc", DataConcernType::Security));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "http://www.IMadeThisUp.com/OpenStuff.aspx&Access_token=abc", DataConcernType::Security));

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "But talking about AWSAccessKey and Signature is not flagged."));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_ContentFormatMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "<HTML><P><Table>", DataConcernType::Content));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "<?xml version=\\\"1.0\\\" encoding=\\\"utf - 8\\\"?><UnifiedRules xmlns=\\\"urn: UnifiedRules\\\" xmlns:xsi=\\\"http://www.w3.org/2001/XMLSchema-instance\\\" xsi:schemaLocation=\\\"urn:UnifiedRules ../../RulesSchema/Schemas/UnifiedRules.xsd\\\">", DataConcernType::Content));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "{\\rtf1\\adeflang1025\\ansi\\ansicpg1252\\uc1\\adeff0\\deff0\\stshfdbch0\\stshfloch39\\stshfhich39\\stshfbi39\\deflang1033\\deflangfe1033\\themelang1033\\themelangfe0\\themelangcs0{\\fonttbl{\\f0\\fbidi \\froman\\fcharset0\\fprq2{\\*\\panose 02020603050405020304}Times New Roman{\\*\\falt Times};}", DataConcernType::Content));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "<asp:TableCell ID=\\\"TableCell10\\\" runat=\\\"server\\\">", DataConcernType::Content));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "MIME-Version:1.0", DataConcernType::Content));

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "HTML rtf xml"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PidKeyMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "1A2B3-C4D5E-6F7H8-I9J0K-LMNOP", DataConcernType::PIDKey));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_UserMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "This content brought to you by awesomeuser and theletterdee", DataConcernType::UserAlias));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Email awesomeuser at microsoft for more info. Don't really, this is just a test", DataConcernType::UserAlias));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "AWESOMEUSER should be flagged.", DataConcernType::UserAlias));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "awesomeuser should be flagged too.", DataConcernType::UserAlias));

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Not expected to find awesome user when case is different to avoid false positives from 'names' in words"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "awesomeusers should not be flagged because the matched alias is part of a longer word"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "lawesomeuser should not be flagged because the matched alias is part of a longer word"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PrettyUserMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "This content brought to you by Awesome Username and theletterdee", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Email Awesome Username at microsoft for more info. Don't really, this is just a test", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome brought free cake today!", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Turns out the cake is a lie. Don't believe Username.", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, c_testUserName, DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Feature=Awesome", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Awesome=Feature", DataConcernType::UserName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Username", DataConcernType::UserName));
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "AwesomeFeature", DataConcernType::UserName));
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "FeatureAwesome", DataConcernType::UserName));

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Many names are commonly seen in word substrings. Like BUDdy, genERIC, etc. awesome username and AWESOME USERNAME shouldn't get flagged."));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PrettyUserMatching_NumbersDoNotCount)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForNumericTest(mockLogger);
    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testNumbers, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"12345.6 stuff"}, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PrettyUserMatching_NonIsolatedWordsNotMatched)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(
        mockLogger,
        "Office Automation Limited Client",
        "dozer",
        c_testDomain,
        c_testComputerName);

    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"Office has an Automation Client. It's Limited."}, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PrettyUserMatching_ShortNamesNotMached)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(
        mockLogger,
        "A Guy",
        "dozer",
        c_testDomain,
        c_testComputerName);

    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"His name is A Guy."}, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"A person named Guy walks down the road."}, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_MachineName)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "MOTHERBOARD should be flagged.", DataConcernType::MachineName));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "motherboard should be flagged too.", DataConcernType::MachineName));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Not expected to find awesome user when case is different to avoid false positives from 'names' in words"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "motherboarding should not be flagged because the matched name is part of a longer word."));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "grandmotherboard should not be flagged because the matched name is part of a longer word."));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_MachineName_NonIntuitiveStrings)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(
        mockLogger,
        c_testUserName,
        PrivacyGuardTests::MakeNonIntuitiveString(),
        c_testDomain,
        PrivacyGuardTests::MakeNonIntuitiveString());

    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, std::string{"Nothing to find here."}, c_testTargetTenant);
    ASSERT_EQ(0, issues.size());

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_DomainName)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "TEST.MICROSOFT.COM should be flagged.", DataConcernType::UserDomain));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "test.microsoft.com should be flagged too.", DataConcernType::UserDomain));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_BannedIdentityTypes)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "*_SSPI"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "*_AD"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_SSPI"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_AD"));

    //SSPI format includes the '@' so will be picked up by the other email-finding code. Not sure about AD.
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email@contoso.com_SSPI", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email@microsoft.com_SSPI", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email@contoso.com_AD", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email@microsoft.com_AD", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email_contoso.com_SSPI", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email_microsoft.com_SSPI", DataConcernType::InternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email_contoso.com_AD", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "email_microsoft.com_AD", DataConcernType::InternalEmailAddress));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_DirectoryMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "C:\\Users\\sunshine\\AppData\\Loca", DataConcernType::Directory));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "C:\\\\Users\\\\sunshine\\\\AppData\\\\Loca", DataConcernType::Directory));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "\\\\Users\\sunshine\\AppData\\Loca", DataConcernType::Directory));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Failure to rain on c:\\\\Users\\\\sunshine", DataConcernType::Directory));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "{ \"JobID\" : \"341729903\",\"ScenarioPath\" : \"OfficeVSO\\\\OC\\\\Word\\\\Core\\\\File IO and Collaboration\\\\RTT\\\\Track Changes\"}", DataConcernType::Directory));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "{UnicodeEscapeFalsePositive:\\u0027formulaODataFeeds\\u0027}"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Diagnostic Context:\\n    Info: 1234\\n"));  //Some warnings in Mac Outlook cause this false positive due to "t:\n".
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Failed to open key HKEY_LOCAL_MACHINE\\\\Software\\\\Microsoft\\\\Office\\\\16.0\\\\Common\\\\Feature"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_ExternalEmailMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Using the seeing stones will send an email alert to Sauron@contoso.com", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "and also cc Saruman@contoso.com", DataConcernType::ExternalEmailAddress));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_LocationMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Location: Moonbuggy", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Latitude: 6deg North", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Longitude: 12deg West", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Coordinates: 5 by 5", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Coord: Castle 5", DataConcernType::Location));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Coordinator: Castle 5"));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "GeoLocation: Pit of despair", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Geo: At the end of the rainbow", DataConcernType::Location));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "GeoID: Shelob's bed & breakfast", DataConcernType::Location));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Geometric: Shelob's bed & breakfast"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_FilesMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.foo", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "NothingToSeeHere.xlsx", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Vacation.JPG", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, ".JPG", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, ".JPG.SomeOtherStuff", DataConcernType::FileNameOrExtension));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "MyAddin.dl"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "TheApp.exe"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common.com"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_PostFixedFilesMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx.CustomCommand", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx.Custom_Command", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_Custom_Command", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_CustomCommand", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "SuperSecretPlans.docx_CustomCommand_", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx.BecauseILikeChopsticks", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "ThereIsNoSpoon.pptx.BecauseILikeChopsticks.", DataConcernType::FileNameOrExtension));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "My_Special.Oddly.Formatted.pptx_item", DataConcernType::FileNameOrExtension));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "My_Special.Oddly.Formatted.free_item"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common."));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, ".Common"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "...Common"));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common..."));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "Common.________.Uncommon"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_InScopeIdentifiers)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_ADAL", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_1::0123456789ABCDEF_ADAL", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_1:live.com:0123456789ABCDEF_ADAL", DataConcernType::InScopeIdentifier));
    ASSERT_FALSE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210_ADAL", DataConcernType::ExternalEmailAddress));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90ab-cdef-fedc-ba9876543210", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "12345678-90AB-CDEF-FEDC-BA9876543210", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef_OrgId", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef_LiveId", DataConcernType::InScopeIdentifier));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "0123456789abcdef", DataConcernType::InScopeIdentifier));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "adal, liveid, orgid, sspi"));

    const char* testGuidsz = "{197648AE-E0E1-4115-962E-29C97E5CD101}";

    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "197648AE-E0E1-4115-962E-29C97E5CD101_ADAL", DataConcernType::InScopeIdentifier));
    auto issues = testPrivacyGuard->GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testGuid, c_testTargetTenant);
    auto issueMatch = std::find_if(issues.cbegin(), issues.cend(), [&](const PrivacyConcern& x) {
        return x.DataConcernType == DataConcernType::InScopeIdentifier && x.EventName == c_testEventName && x.FieldName == c_testFieldName && x.FieldValue == testGuidsz;
    });

    ASSERT_EQ(issues.cend(), issueMatch);

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_DemographicsMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "I English good speak", DataConcernType::DemographicInfoLanguage));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "The app is set to EN-US", DataConcernType::DemographicInfoLanguage));
    ASSERT_TRUE(PrivacyGuardTests::IsExpectedDataConcern(testPrivacyGuard, "Made in the United States", DataConcernType::DemographicInfoCountryRegion));
    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "The ancient tablet States: United the lost 8 pieces of sandwitch to achieve ultimate lunch!"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_OutOfScopeIdentifiersMatching)
{
    auto mockLogger = new MockLogger();
    const auto& testPrivacyGuard = PrivacyGuardTests::GetPrivacyGuardForTest(mockLogger);
    PrivacyGuardTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testClientId);
    PrivacyGuardTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testSqmId);
    PrivacyGuardTests::ValidateOutOfScopeIdentifierIsFlagged(testPrivacyGuard, c_testSusClientId);

    ASSERT_FALSE(PrivacyGuardTests::IdentifiedAnyDataConcerns(testPrivacyGuard, "cbfd6749-165c-41c8-a85e-b9c8b8c1f9ce"));

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_CustomStringValueInspector_CalledCorrectly)
{
    auto mockLogger = new MockLogger();
    TestPrivacyGuard pgInstance(mockLogger, nullptr);

    const std::string testValue{"Foo Test Value"};
    auto customStringValueInspectorCalled = false;
    auto expectedValue = false;
    std::function<DataConcernType(const std::string& valueToInspect, const std::string& tenantToken)> customStringInspector =
        [&customStringValueInspectorCalled, &testValue, &expectedValue](const std::string& valueToInspect, const std::string& /*tenantToken*/) noexcept {
            customStringValueInspectorCalled = true;
            expectedValue = valueToInspect.compare(testValue) == 0;
            return DataConcernType::Content;
        };

    pgInstance.AddCustomStringValueInspector(std::move(customStringInspector));
    auto concerns = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, testValue, c_testTargetTenant);
    ASSERT_TRUE(customStringValueInspectorCalled);
    ASSERT_TRUE(expectedValue);
    ASSERT_EQ(1, concerns.size());
    ASSERT_EQ(concerns[0].DataConcernType, DataConcernType::Content);

    delete mockLogger;
}

TEST(PrivacyGuardTests, GetAllConcerns_CustomGuidValueInspector_CalledCorrectly)
{
    auto mockLogger = new MockLogger();
    TestPrivacyGuard pgInstance(mockLogger, nullptr);

    auto customGuidValueInspectorCalled = false;

    std::function<DataConcernType(GUID_t valueToInspect, const std::string& tenantToken)> customGuidInspector =
        [&customGuidValueInspectorCalled](GUID_t /*valueToInspect*/, const std::string& /*tenantToken*/) noexcept {
            customGuidValueInspectorCalled = true;
            return DataConcernType::Content;
        };

    pgInstance.AddCustomGuidValueInspector(std::move(customGuidInspector));
    auto concerns = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testGuid, c_testTargetTenant);
    ASSERT_TRUE(customGuidValueInspectorCalled);
    ASSERT_EQ(1, concerns.size());
    ASSERT_EQ(concerns[0].DataConcernType, DataConcernType::Content);

    delete mockLogger;
}

TEST(PrivacyGuardTests, FileTypes_IsRegisteredFileType)
{
    ASSERT_TRUE(FileTypes::IsRegisteredFileType(".DOCX"));
    ASSERT_TRUE(FileTypes::IsRegisteredFileType(".PPTX"));
    ASSERT_TRUE(FileTypes::IsRegisteredFileType(".TXT"));
    ASSERT_TRUE(FileTypes::IsRegisteredFileType(".XLSX"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType(".AAA"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType(".ABC"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType(".ZYX"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType("TXT"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType(".Hit_Too_Long_To_Be_A_File_Case"));
    ASSERT_FALSE(FileTypes::IsRegisteredFileType(""));
}

TEST(PrivacyGuardTests, FileTypes_FileTypeListIsValid)
{
    ASSERT_TRUE(FileTypes::IsFileListValid());
}

TEST(PrivacyGuardTests, InspectSemanticContext_CheckStringValue_NotifiesIssueCorrectly)
{
    auto mockLogger = new MockLogger();
    auto logEventCalled = false;
    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
        ASSERT_EQ(properties.GetName(), PrivacyGuard::PrivacyConcernEventName);
        auto props = properties.GetProperties();
        auto type = props.find("TypeAsText");
        ASSERT_NE(type, props.end());
        ASSERT_EQ(type->second.as_string, PrivacyGuard::DataConcernTypeAsText(DataConcernType::ExternalEmailAddress));
        logEventCalled = true;
    };
    PrivacyGuard pgInstance(mockLogger);
    pgInstance.InspectSemanticContext(c_testFieldName, c_testEmail, true, c_testTargetTenant);
    ASSERT_TRUE(logEventCalled);

    delete mockLogger;
}

TEST(PrivacyGuardTests, InspectSemanticContext_CheckGuidValue_NotifiesIssueCorrectly)
{
    auto mockLogger = new MockLogger();
    auto logEventCalled = false;
    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
        ASSERT_EQ(properties.GetName(), PrivacyGuard::PrivacyConcernEventName);
        auto props = properties.GetProperties();
        auto type = props.find("TypeAsText");
        ASSERT_NE(type, props.end());
        ASSERT_EQ(type->second.as_string, PrivacyGuard::DataConcernTypeAsText(DataConcernType::InScopeIdentifier));
        logEventCalled = true;
    };
    PrivacyGuard pgInstance(mockLogger);
    pgInstance.InspectSemanticContext(c_testFieldName, c_testAdalGuid, true, c_testTargetTenant);
    ASSERT_TRUE(logEventCalled);
    logEventCalled = false;
    pgInstance.InspectSemanticContext(c_testFieldName, c_testGuid, true, c_testTargetTenant);
    ASSERT_TRUE(logEventCalled);

    delete mockLogger;
}

TEST(PrivacyGuardTests, InspectSemanticContext_IgnoredConcern_DoesNotNotifyIssue)
{
    auto mockLogger = new MockLogger();
    auto logEventCalled = false;
    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& /*properties*/) noexcept {
        logEventCalled = true;
    };
    TestPrivacyGuard pgInstance(mockLogger, nullptr);

    std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>> ignoredConcern;
    ignoredConcern.push_back(std::make_tuple(c_testEventName, c_testFieldName, DataConcernType::InScopeIdentifier));

    pgInstance.AddIgnoredConcern(ignoredConcern);
    auto results = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testAdalGuid, c_testTargetTenant);
    ASSERT_FALSE(logEventCalled);
    logEventCalled = false;
    results = pgInstance.GetAllPrivacyConcerns(c_testEventName, c_testFieldName, c_testAdalGuid, c_testTargetTenant);
    pgInstance.InspectSemanticContext(c_testFieldName, c_testGuid, true, c_testTargetTenant);
    ASSERT_FALSE(logEventCalled);

    delete mockLogger;
}

TEST(PrivacyGuardTests, Decorate_InspectStringAndGuidData)
{
    auto mockLogger = new MockLogger();
    auto logEventCalled = false;
    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& /*properties*/) noexcept {
        logEventCalled = true;
    };
    PrivacyGuard pgInstance(mockLogger);

    auto testRecord = PrivacyGuardTests::GenerateTestRecord(c_testAdalGuid);
    ASSERT_TRUE(pgInstance.decorate(testRecord));
    ASSERT_TRUE(logEventCalled);

    delete mockLogger;
}

TEST(PrivacyGuardTests, Decorate_MultipleEventsAddInscopeIdentifier_InspectStringAndGuidData)
{
    auto mockLogger = new MockLogger();
    auto logEventCalled = false;
    mockLogger->m_logEventOverride = [&logEventCalled](const EventProperties& /*properties*/) noexcept {
        logEventCalled = true;
    };
    PrivacyGuard pgInstance(mockLogger);

    auto testRecord = PrivacyGuardTests::GenerateTestRecord(c_testAdalGuid);
    ASSERT_TRUE(pgInstance.decorate(testRecord));
    ASSERT_TRUE(pgInstance.decorate(testRecord));
    ASSERT_TRUE(logEventCalled);

    delete mockLogger;
}

#endif
// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"

#define HAVE_MAT_PRIVACYGUARD

#if defined __has_include
#if __has_include("modules/privacyguard/PrivacyGuard.hpp")
#include "modules/privacyguard/PrivacyGuard.hpp"
#else
/* Compiling without Privacy Guard */
#undef HAVE_MAT_PRIVACYGUARD
#endif
#endif

#ifdef HAVE_MAT_PRIVACYGUARD

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
#define TEST_TOKEN "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

class PrivacyGuardFuncTests : public ::testing::Test
{
   public:
    static CommonDataContexts GenerateTestDataContexts() noexcept
    {
        return GenerateTestDataContexts("", "", "", "");
    }

    static CommonDataContexts GenerateTestDataContexts(const std::string& userName,
                                                       const std::string& userAlias,
                                                       const std::string& domainName,
                                                       const std::string& machineName) noexcept
    {
        CommonDataContexts cdc;
        if (userName.empty())
        {
            cdc.UserName = c_testUserName;
        }
        else
        {
            cdc.UserName = userName;
        }

        if (userAlias.empty())
        {
            cdc.UserAlias = c_testUserAlias;
        }
        else
        {
            cdc.UserAlias = userAlias;
        }
        if (domainName.empty())
        {
            cdc.DomainName = c_testDomain;
        }
        else
        {
            cdc.DomainName = domainName;
        }

        if (machineName.empty())
        {
            cdc.MachineName = c_testComputerName;
        }
        else
        {
            cdc.MachineName = machineName;
        }
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

    virtual void SetUp() override
    {
        const std::shared_ptr<IDataInspector> privacyGuard = std::make_shared<PrivacyGuard>(&m_mockLogger, nullptr);
        LogManager::Initialize();
        LogManager::GetInstance()->SetDataInspector(privacyGuard);
    }

    virtual void TearDown() override
    {
        LogManager::GetInstance()->SetDataInspector(nullptr);
    }

    static std::shared_ptr<IDataInspector> InitializePrivacyGuardWithCustomLoggerAndDataContext(ILogger* logger, std::unique_ptr<CommonDataContexts> context) noexcept
    {
        const std::shared_ptr<IDataInspector> privacyGuard = std::make_shared<PrivacyGuard>(logger, std::move(context));
        LogManager::GetInstance()->SetDataInspector(privacyGuard);

        return privacyGuard;
    }

    MockLogger m_mockLogger;
};

TEST_F(PrivacyGuardFuncTests, InitializePrivacyGuard)
{
    LogManager::Initialize();
    LogManager::GetInstance()->SetDataInspector(nullptr);
    ASSERT_NO_THROW([]() {
        MockLogger mockLogger;
        const std::shared_ptr<IDataInspector> privacyGuard = std::make_shared<PrivacyGuard>(&mockLogger, nullptr);
        LogManager::GetInstance()->SetDataInspector(privacyGuard);
        ASSERT_TRUE(LogManager::GetInstance()->GetDataInspector() != nullptr);
    });
    ASSERT_NO_THROW([]() {
        MockLogger mockLogger;
        const std::shared_ptr<IDataInspector> privacyGuard = std::make_shared<PrivacyGuard>(&mockLogger, std::move(std::make_unique<CommonDataContexts>()));
        LogManager::GetInstance()->SetDataInspector(privacyGuard);
        ASSERT_TRUE(LogManager::GetInstance()->GetDataInspector() != nullptr);
    });
}

TEST_F(PrivacyGuardFuncTests, SetDataInspectorState)
{
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspector()->IsEnabled());
    LogManager::GetInstance()->GetDataInspector()->SetEnabled(/*isEnabled:*/ false);
    ASSERT_FALSE(LogManager::GetInstance()->GetDataInspector()->IsEnabled());
    LogManager::GetInstance()->GetDataInspector()->SetEnabled(/*isEnabled:*/ true);
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspector()->IsEnabled());
}

TEST_F(PrivacyGuardFuncTests, SetCommonDataContexts)
{
    ASSERT_TRUE(LogManager::GetInstance()->GetDataInspector()->IsEnabled());
    LogManager::GetInstance()->GetDataInspector()->AppendCommonDataContext(std::move(std::make_unique<CommonDataContexts>()));
}

TEST_F(PrivacyGuardFuncTests, AddIgnoredDataConcern)
{
    std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>> ignoredConcerns;
    ignoredConcerns.push_back(std::make_tuple(c_testEventName, c_testFieldName, DataConcernType::InScopeIdentifier));
    LogManager::GetInstance()->GetDataInspector()->AddIgnoredConcern(ignoredConcerns);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyEmail_FoundEmail)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Some%2eone%40Microsoft%2ecom");     //ConcernType::InternalEmailAddress  //As happens in escaped URLs
    props.SetProperty("Field2", "Someone@Microsoft.com");            //ConcernType::InternalEmailAddress
    props.SetProperty("Field3", "Some.one@Exchange.Microsoft.com");  //ConcernType::InternalEmailAddress
    props.SetProperty("Field4", "Some_one@microsoft_com");           //ConcernType::InternalEmailAddress
    props.SetProperty("Field5", "Some_one_AT_microsoft_com");        //ConcernType::InternalEmailAddress
    props.SetProperty("Field6", "Microsoft.com");
    props.SetProperty("Field7", "Exchange.Microsoft.com");
    props.SetProperty("Field8", "Some_one");
    logger->LogEvent(props);
    ASSERT_EQ(5, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyUrl_FoundUrl)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "http://www.microsoft.com");                             //DataConcernType::Url
    props.SetProperty("Field2", "HTTPS://www.microsoft.com");                            //DataConcernType::Url
    props.SetProperty("Field3", "File://www.microsoft.com");                             //DataConcernType::Url & DataConcernType::FileSharingUrl
    props.SetProperty("Field4", "Download failed for domain https://wopi.dropbox.com");  //DataConcernType::Url
    logger->LogEvent(props);
    ASSERT_EQ(5, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyFileSharingUrl_FoundFileSharingUrl)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;

    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "http://www.dropbox.com/aaaaa");     //DataConcernType::FileSharingUrl & DataConcernType::Url
    props.SetProperty("Field2", "HTTPS://wopi.dropbox.com/aaaaaa");  //DataConcernType::FileSharingUrl & DataConcernType::Url
    props.SetProperty("Field3", "File://loggingShare/MyLogFolder");  //DataConcernType::FileSharingUrl & DataConcernType::Url
    props.SetProperty(
        "Field4",
        "Having https://wopi.dropbox.com/ appear in the middle of text is expected to confuse the detector even though there's no file encoding");  //DataConcernType::FileSharingUrl
    props.SetProperty("Field5", "But talking about http and dropbox.com/ should not.");

    props.SetProperty("Field6", "https://outlook.live.com/owa/wopi/files/[guid]@outlook.com/[encoded_goo]");       //DataConcernType::FileSharingUrl & DataConcernType::Url
    props.SetProperty("Field7", "https://outlook.live.com/owa/wopi.ashx/files/[guid]@outlook.com/[encoded_goo]");  //DataConcernType::FileSharingUrl & DataConcernType::Url

    //Not file-sharing if just talking about the domain.
    props.SetProperty("Field8", "Download failed for domain https://wopi.dropbox.com/");  //DataConcernType::FileSharingUrl & DataConcernType::Url
    props.SetProperty("Field9", "Download failed for domain https://wopi.dropbox.com");   //DataConcernType::Url
    logger->LogEvent(props);
    ASSERT_EQ(14, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_CheckForSecurity_SecurityChecked)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "http://www.dropbox.com/aaaaa");                                  //DataConcernType::FileSharingUrl & DataConcernType::URL
    props.SetProperty("Field2", "http://www.IMadeThisUp.com/OpenStuff.aspx&AWSAccessKeyId=abc");  //DataConcernType::Security & DataConcernType::URL
    props.SetProperty("Field3", "http://www.IMadeThisUp.com/OpenStuff.aspx&Signature=abc");       //DataConcernType::Security & DataConcernType::URL
    props.SetProperty("Field4", "http://www.IMadeThisUp.com/OpenStuff.aspx&Access_token=abc");    //DataConcernType::Security & DataConcernType::URL

    props.SetProperty("Field5", "But talking about AWSAccessKey and Signature is not flagged.");
    logger->LogEvent(props);
    ASSERT_EQ(8, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyContentFormat_FoundContentFormat)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "<HTML><P><Table>");  //DataConcernType::Content
    props.SetProperty(
        "Field2",
        "<?xml version=\\\"1.0\\\" encoding=\\\"utf - 8\\\"?><UnifiedRules xmlns=\\\"urn: UnifiedRules\\\" xmlns:xsi=\\\"http://www.w3.org/2001/XMLSchema-instance\\\" xsi:schemaLocation=\\\"urn:UnifiedRules ../../RulesSchema/Schemas/UnifiedRules.xsd\\\">");  //DataConcernType::Content, DataConcernType::URL, DataConcernType::Location

    props.SetProperty(
        "Field3",
        "{\\rtf1\\adeflang1025\\ansi\\ansicpg1252\\uc1\\adeff0\\deff0\\stshfdbch0\\stshfloch39\\stshfhich39\\stshfbi39\\deflang1033\\deflangfe1033\\themelang1033\\themelangfe0\\themelangcs0{\\fonttbl{\\f0\\fbidi \\froman\\fcharset0\\fprq2{\\*\\panose 02020603050405020304}Times New Roman{\\*\\falt Times};}");  //DataConcernType::Content

    props.SetProperty(
        "Field4",
        "<asp:TableCell ID=\\\"TableCell10\\\" runat=\\\"server\\\">");  //DataConcernType::Content

    props.SetProperty(
        "Field5",
        "MIME-Version:1.0");  //DataConcernType::Content

    props.SetProperty("Field6", "HTML rtf xml");
    logger->LogEvent(props);
    ASSERT_EQ(7, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPidKey_FoundPidKey)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "1A2B3-C4D5E-6F7H8-I9J0K-LMNOP");  //DataConcernType::PIDKey
    logger->LogEvent(props);
    ASSERT_EQ(1, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyUser_FoundUser)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "This content brought to you by awesomeuser and theletterdee");                      //DataConcernType::UserAlias
    props.SetProperty("Field2", "Email awesomeuser at microsoft for more info. Don't really, this is just a test");  //DataConcernType::UserAlias
    props.SetProperty("Field3", "AWESOMEUSER should be flagged.");                                                   //DataConcernType::UserAlias
    props.SetProperty("Field4", "awesomeuser should be flagged too.");                                               //DataConcernType::UserAlias

    props.SetProperty("Field5", "Not expected to find awesome user when case is different to avoid false positives from 'names' in words");
    props.SetProperty("Field6", "awesomeusers should not be flagged because the matched alias is part of a longer word");
    props.SetProperty("Field7", "lawesomeuser should not be flagged because the matched alias is part of a longer word");
    logger->LogEvent(props);
    ASSERT_EQ(4, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPrettyUser_FoundPrettyUser)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "This content brought to you by Awesome Username and theletterdee");                      //DataConcernType::UserName
    props.SetProperty("Field2", "Email Awesome Username at microsoft for more info. Don't really, this is just a test");  //DataConcernType::UserName
    props.SetProperty("Field3", "Awesome brought free cake today!");                                                      //DataConcernType::UserName
    props.SetProperty("Field4", "Turns out the cake is a lie. Don't believe Username.");                                  //DataConcernType::UserName
    props.SetProperty("Field5", c_testUserName);                                                                          //DataConcernType::UserName
    props.SetProperty("Field6", "Awesome");                                                                               //DataConcernType::UserName
    props.SetProperty("Field7", "Feature=Awesome");                                                                       //DataConcernType::UserName
    props.SetProperty("Field8", "Awesome=Feature");                                                                       //DataConcernType::UserName
    props.SetProperty("Field9", "Username");                                                                              //DataConcernType::UserName
    props.SetProperty("Field10", "AwesomeFeature");
    props.SetProperty("Field11", "FeatureAwesome");

    props.SetProperty("Field12", "Many names are commonly seen in word substrings. Like BUDdy, genERIC, etc. awesome username and AWESOME USERNAME shouldn't get flagged.");
    logger->LogEvent(props);
    ASSERT_EQ(9, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyMachineName_MachineNameFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "MOTHERBOARD should be flagged.");      //DataConcernType::MachineName
    props.SetProperty("Field2", "motherboard should be flagged too.");  //DataConcernType::MachineName
    props.SetProperty("Field3", "Not expected to find awesome user when case is different to avoid false positives from 'names' in words");
    props.SetProperty("Field4", "motherboarding should not be flagged because the matched name is part of a longer word.");
    props.SetProperty("Field5", "grandmotherboard should not be flagged because the matched name is part of a longer word.");
    logger->LogEvent(props);
    ASSERT_EQ(2, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPrettyUser_NumbersDoNotCount)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto cdc = std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts(
        "",
        c_testNumbers,
        c_testNumbers,
        c_testNumbers));

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(cdc));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", c_testNumbers);
    props.SetProperty("Field2", "12345.6 stuff");

    logger->LogEvent(props);
    ASSERT_EQ(0, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPrettyUser_NonIsolatedWordsNotMatched)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto cdc = std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts(
        std::string{"Office Automation Limited Client"},
        std::string{"dozer"},
        c_testDomain,
        c_testComputerName));

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(cdc));

    EventProperties props(c_testEventName);

    props.SetProperty("Field1", "Office has an Automation Client. It's Limited.");

    logger->LogEvent(props);
    ASSERT_EQ(0, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPrettyUser_ShortNamesNotMached)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto cdc = std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts(
        "A Guy",
        "dozer",
        c_testDomain,
        c_testComputerName));

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(cdc));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "His name is A Guy.");
    props.SetProperty("Field2", "A person named Guy walks down the road.");
    logger->LogEvent(props);
    ASSERT_EQ(0, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyMachineNameInNonIntuitiveStrings_MachineNameNotFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto cdc = std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts(
        c_testUserName,
        PrivacyGuardFuncTests::MakeNonIntuitiveString(),
        c_testDomain,
        PrivacyGuardFuncTests::MakeNonIntuitiveString()));

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(cdc));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Nothing to find here.");
    logger->LogEvent(props);
    ASSERT_EQ(0, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyDomainName_DomainNameFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "TEST.MICROSOFT.COM should be flagged.");      //DataConcernType::UserDomain
    props.SetProperty("Field2", "test.microsoft.com should be flagged too.");  //DataConcernType::UserDomain

    logger->LogEvent(props);
    ASSERT_EQ(2, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyBannedIdentityTypes_BannedIdentityTypesFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "*_SSPI");
    props.SetProperty("Field2", "*_AD");
    props.SetProperty("Field3", "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_SSPI");
    props.SetProperty("Field4", "OMEX,ListOfInScopeIdentifiers;EXCatalog,*_AD");

    //SSPI format includes the '@' so will be picked up by the other email-finding code. Not sure about AD.
    props.SetProperty("Field5", "email@contoso.com_SSPI");     //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field6", "email@microsoft.com_SSPI");   //DataConcernType::InternalEmailAddress
    props.SetProperty("Field7", "email@contoso.com_AD");       //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field8", "email@microsoft.com_AD");     //DataConcernType::InternalEmailAddress
    props.SetProperty("Field9", "email_contoso.com_SSPI");     //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field10", "email_microsoft.com_SSPI");  //DataConcernType::InternalEmailAddress
    props.SetProperty("Field11", "email_contoso.com_AD");      //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field12", "email_microsoft.com_AD");    //DataConcernType::InternalEmailAddress

    logger->LogEvent(props);
    ASSERT_EQ(8, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyDirectory_FoundDirectory)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "C:\\Users\\sunshine\\AppData\\Loca");                                                                                                        //DataConcernType::Directory
    props.SetProperty("Field2", "C:\\\\Users\\\\sunshine\\\\AppData\\\\Loca");                                                                                                //DataConcernType::Directory
    props.SetProperty("Field3", "\\\\Users\\sunshine\\AppData\\Loca");                                                                                                        //DataConcernType::Directory
    props.SetProperty("Field4", "Failure to rain on c:\\\\Users\\\\sunshine");                                                                                                //DataConcernType::Directory
    props.SetProperty("Field5", "{ \"JobID\" : \"341729903\",\"ScenarioPath\" : \"OfficeVSO\\\\OC\\\\Word\\\\Core\\\\File IO and Collaboration\\\\RTT\\\\Track Changes\"}");  //DataConcernType::Directory
    props.SetProperty("Field6", "{UnicodeEscapeFalsePositive:\\u0027formulaODataFeeds\\u0027}");
    props.SetProperty("Field7", "Diagnostic Context:\\n    Info: 1234\\n");  //Some warnings in Mac Outlook cause this false positive due to "t:\n".
    props.SetProperty("Field8", "Failed to open key HKEY_LOCAL_MACHINE\\\\Software\\\\Microsoft\\\\Office\\\\16.0\\\\Common\\\\Feature");

    logger->LogEvent(props);
    ASSERT_EQ(5, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyExternalEmail_FoundExternalEmail)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Using the seeing stones will send an email alert to Sauron@contoso.com");  //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field2", "and also cc Saruman@contoso.com");                                         //DataConcernType::ExternalEmailAddress

    logger->LogEvent(props);
    ASSERT_EQ(2, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyLocation_FoundLocation)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Location: Moonbuggy");    //DataConcernType::Location
    props.SetProperty("Field2", "Latitude: 6deg North");   //DataConcernType::Location
    props.SetProperty("Field3", "Longitude: 12deg West");  //DataConcernType::Location
    props.SetProperty("Field4", "Coordinates: 5 by 5");    //DataConcernType::Location
    props.SetProperty("Field5", "Coord: Castle 5");        //DataConcernType::Location
    props.SetProperty("Field6", "Coordinator: Castle 5");
    props.SetProperty("Field7", "GeoLocation: Pit of despair");      //DataConcernType::Location
    props.SetProperty("Field8", "Geo: At the end of the rainbow");   //DataConcernType::Location
    props.SetProperty("Field9", "GeoID: Shelob's bed & breakfast");  //DataConcernType::Location
    props.SetProperty("Field10", "Geometric: Shelob's bed & breakfast");

    logger->LogEvent(props);
    ASSERT_EQ(8, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyFiles_FoundFiles)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "SuperSecretPlans.foo");
    props.SetProperty("Field2", "SuperSecretPlans.docx");  //DataConcernType::FileNameOrExtension
    props.SetProperty("Field3", "NothingToSeeHere.xlsx");  //DataConcernType::FileNameOrExtension
    props.SetProperty("Field4", "ThereIsNoSpoon.pptx");    //DataConcernType::FileNameOrExtension
    props.SetProperty("Field5", "Vacation.JPG");           //DataConcernType::FileNameOrExtension
    props.SetProperty("Field6", ".JPG");                   //DataConcernType::FileNameOrExtension
    props.SetProperty("Field7", ".JPG.SomeOtherStuff");    //DataConcernType::FileNameOrExtension
    props.SetProperty("Field8", "MyAddin.dl");
    props.SetProperty("Field9", "TheApp.exe");
    props.SetProperty("Field10", "Common.com");

    logger->LogEvent(props);
    ASSERT_EQ(6, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyPostFixedFiles_FoundPostFixedFiles)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "SuperSecretPlans.docx.CustomCommand");          //DataConcernType::FileNameOrExtension
    props.SetProperty("Field2", "SuperSecretPlans.docx.Custom_Command");         //DataConcernType::FileNameOrExtension
    props.SetProperty("Field3", "SuperSecretPlans.docx_Custom_Command");         //DataConcernType::FileNameOrExtension
    props.SetProperty("Field4", "SuperSecretPlans.docx_CustomCommand");          //DataConcernType::FileNameOrExtension
    props.SetProperty("Field5", "SuperSecretPlans.docx_CustomCommand_");         //DataConcernType::FileNameOrExtension
    props.SetProperty("Field6", "ThereIsNoSpoon.pptx.BecauseILikeChopsticks");   //DataConcernType::FileNameOrExtension
    props.SetProperty("Field7", "ThereIsNoSpoon.pptx.BecauseILikeChopsticks.");  //DataConcernType::FileNameOrExtension
    props.SetProperty("Field8", "My_Special.Oddly.Formatted.pptx_item");         //DataConcernType::FileNameOrExtension
    props.SetProperty("Field9", "My_Special.Oddly.Formatted.free_item");
    props.SetProperty("Field10", "Common.");
    props.SetProperty("Field11", ".Common");
    props.SetProperty("Field12", "...Common");
    props.SetProperty("Field13", "Common...");
    props.SetProperty("Field14", "Common.________.Uncommon");

    logger->LogEvent(props);
    ASSERT_EQ(8, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyInScopeIdentifiers_InScopeIdentifiersFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "12345678-90ab-cdef-fedc-ba9876543210_ADAL");                              //DataConcernType::InScopeIdentifier
    props.SetProperty("Field2", "12345678-90ab-cdef-fedc-ba9876543210_1::0123456789ABCDEF_ADAL");          //DataConcernType::InScopeIdentifier
    props.SetProperty("Field3", "12345678-90ab-cdef-fedc-ba9876543210_1:live.com:0123456789ABCDEF_ADAL");  //DataConcernType::InScopeIdentifier
    props.SetProperty("Field4", "12345678-90ab-cdef-fedc-ba9876543210_ADAL");                              //DataConcernType::ExternalEmailAddress
    props.SetProperty("Field5", "12345678-90ab-cdef-fedc-ba9876543210");                                   //DataConcernType::InScopeIdentifier
    props.SetProperty("Field6", "12345678-90AB-CDEF-FEDC-BA9876543210");                                   //DataConcernType::InScopeIdentifier
    props.SetProperty("Field7", "0123456789abcdef_OrgId");                                                 //DataConcernType::InScopeIdentifier
    props.SetProperty("Field8", "0123456789abcdef_LiveId");                                                //DataConcernType::InScopeIdentifier
    props.SetProperty("Field9", "0123456789abcdef");                                                       //DataConcernType::InScopeIdentifier
    props.SetProperty("Field10", "197648AE-E0E1-4115-962E-29C97E5CD101_ADAL");                             //DataConcernType::InScopeIdentifier
    props.SetProperty("Field11", "adal, liveid, orgid, sspi");

    logger->LogEvent(props);
    ASSERT_EQ(10, privacyConcernLogCount);

    privacyConcernLogCount = 0;

    EventProperties props2(c_testEventName);
    props2.SetProperty("Field1", c_testGuid);

    logger->LogEvent(props2);
    ASSERT_EQ(1, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyDemographics_FoundDemographics)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "I English good speak");       //DataConcernType::DemographicInfoLanguage
    props.SetProperty("Field2", "The app is set to EN-US");    //DataConcernType::DemographicInfoLanguage
    props.SetProperty("Field3", "Made in the United States");  //DataConcernType::DemographicInfoCountryRegion
    props.SetProperty("Field4", "The ancient tablet States: United the lost 8 pieces of sandwitch to achieve ultimate lunch!");
    logger->LogEvent(props);
    ASSERT_EQ(3, privacyConcernLogCount);
}

std::string GenerateIdentifierVariant(const std::string& input, bool uppercase, bool removeDashes)
{
    std::string value;

    if (input.length() >= 256)
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

void ValidateOutOfScopeIdentifierIsFlagged(ILogger* logger, const std::string& identifier)
{
    EventProperties props(c_testEventName);

    ASSERT_TRUE(identifier.length() < 256);
    props.SetProperty("Field1", identifier);  //DataConcernType::OutOfScopeIdentifier

    //Should find case insensitive and with or without dashes in GUIDs
    props.SetProperty("Field2", GenerateIdentifierVariant(identifier, true, false));  // DataConcernType::OutOfScopeIdentifier
    props.SetProperty("Field3", GenerateIdentifierVariant(identifier, false, true));  // DataConcernType::OutOfScopeIdentifier
    props.SetProperty("Field4", GenerateIdentifierVariant(identifier, true, true));   // DataConcernType::OutOfScopeIdentifier
    logger->LogEvent(props);
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyOutOfScopeIdentifiers_FoundOutOfScopeIdentifiers)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    ValidateOutOfScopeIdentifierIsFlagged(logger, c_testClientId);
    ASSERT_EQ(4, privacyConcernLogCount);
    privacyConcernLogCount = 0;
    ValidateOutOfScopeIdentifierIsFlagged(logger, c_testSqmId);
    ASSERT_EQ(4, privacyConcernLogCount);
    privacyConcernLogCount = 0;
    ValidateOutOfScopeIdentifierIsFlagged(logger, c_testSusClientId);
    ASSERT_EQ(4, privacyConcernLogCount);
    privacyConcernLogCount = 0;
}

TEST_F(PrivacyGuardFuncTests, LogEvent_IdentifyOutOfScopeIdentifiers_RandmonGuidNotMatched)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    mockLogger.m_logEventOverride = [&privacyConcernLogCount](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCount++;
        }
    };

    auto logger = LogManager::GetLogger(TEST_TOKEN);
    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, std::move(std::make_unique<CommonDataContexts>(PrivacyGuardFuncTests::GenerateTestDataContexts())));

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "cbfd6749-165c-41c8-a85e-b9c8b8c1f9ce");
    logger->LogEvent(props);
    ASSERT_EQ(0, privacyConcernLogCount);
}

TEST_F(PrivacyGuardFuncTests, InspectSemanticContext_CheckContextValues_NotifiesIssueCorrectly)
{
    MockLogger mockLogger;
    auto logEventCalled = false;
    mockLogger.m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            logEventCalled = true;
        }
    };

    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    LogManager::SetContext(c_testFieldName, c_testEmail);
    ASSERT_TRUE(logEventCalled);
    logEventCalled = false;
    LogManager::SetContext(c_testFieldName, c_testAdalGuid);
    ASSERT_TRUE(logEventCalled);
    LogManager::SetContext(c_testFieldName, c_testGuid);
    ASSERT_TRUE(logEventCalled);
}

TEST_F(PrivacyGuardFuncTests, InspectSemanticContext_IgnoredConcern_NoNotificationFired)
{
    MockLogger mockLogger;
    auto logEventCalled = false;
    mockLogger.m_logEventOverride = [&logEventCalled](const EventProperties& properties) noexcept {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            logEventCalled = true;
        }
    };

    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);

    std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>> ignoredConcern;
    ignoredConcern.push_back(std::make_tuple(c_testEventName, c_testFieldName, DataConcernType::ExternalEmailAddress));
    pg->AddIgnoredConcern(ignoredConcern);

    LogManager::SetContext(c_testFieldName, c_testEmail);
    ASSERT_FALSE(logEventCalled);
}

TEST_F(PrivacyGuardFuncTests, PrivacyGuardDisabled_IdentifyEmail_NothingFound)
{
    MockLogger mockLogger;
    auto privacyConcernLogCalled = false;
    mockLogger.m_logEventOverride = [&privacyConcernLogCalled](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), PrivacyGuard::PrivacyConcernEventName))
        {
            privacyConcernLogCalled = true;
        }
    };

    auto pg = PrivacyGuardFuncTests::InitializePrivacyGuardWithCustomLoggerAndDataContext(&mockLogger, nullptr);
    auto logger = LogManager::GetLogger(TEST_TOKEN);
    pg->SetEnabled(false);
    ASSERT_FALSE(LogManager::GetInstance()->GetDataInspector()->IsEnabled());

    EventProperties props(c_testEventName);
    props.SetProperty("Field1", "Some%2eone%40Microsoft%2ecom");     //ConcernType::InternalEmailAddress  //As happens in escaped URLs
    props.SetProperty("Field2", "Someone@Microsoft.com");            //ConcernType::InternalEmailAddress
    props.SetProperty("Field3", "Some.one@Exchange.Microsoft.com");  //ConcernType::InternalEmailAddress
    props.SetProperty("Field4", "Some_one@microsoft_com");           //ConcernType::InternalEmailAddress
    props.SetProperty("Field5", "Some_one_AT_microsoft_com");        //ConcernType::InternalEmailAddress
    props.SetProperty("Field6", "Microsoft.com");
    props.SetProperty("Field7", "Exchange.Microsoft.com");
    props.SetProperty("Field8", "Some_one");
    logger->LogEvent(props);
    ASSERT_FALSE(privacyConcernLogCalled);
}
#else

#endif

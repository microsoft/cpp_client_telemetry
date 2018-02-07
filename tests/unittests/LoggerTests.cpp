// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "api/Logger.hpp"
#include "common/MockILogManagerInternal.hpp"
#include "common/MockIRuntimeConfig.hpp"

using namespace testing;
using namespace ARIASDK_NS;


class Logger4Test : public Logger {
  public:
    template<typename... TArgs>
    Logger4Test(TArgs&& ... args)
      : Logger(std::forward<TArgs>(args) ...)
    {
    }

    MOCK_METHOD4(submit, void(::AriaProtocol::CsEvent &, 
                              ::Microsoft::Applications::Events::EventLatency,
                              ::Microsoft::Applications::Events::EventPersistence, 
                              std::uint64_t  const& ));

    void submit_(::AriaProtocol::CsEvent& record, ::Microsoft::Applications::Events::EventLatency latency, ::Microsoft::Applications::Events::EventPersistence persistence, std::uint64_t  const& policyBitFlags)
    {
        return Logger::submit(record, latency, persistence, policyBitFlags);
    }
};


class LoggerTests : public Test {
  protected:
    StrictMock<MockIRuntimeConfig>      _runtimeConfigMock;
    StrictMock<MockILogManagerInternal>         _logManagerMock;
    Logger4Test                         _logger;

    EventProperties                     _emptyProperties;

    bool                                _submitted;
    ::AriaProtocol::CsEvent              _submittedRecord;
    EventLatency                         _submittedLatency;

    uint64_t                            _sequenceId;

  protected:
    LoggerTests()
      : _logger("testtenantid-tenanttoken", "test-source", "ecs-project", &_logManagerMock, nullptr, &_runtimeConfigMock),
        _emptyProperties("")
    {
    }

    static void fakeRuntimeConfigDecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName)
    {
        UNREFERENCED_PARAMETER(experimentationProject);
        UNREFERENCED_PARAMETER(eventName);

        extension["runtimeconfigvalue"] = "blah";
    }

    virtual void SetUp() override
    {
        EXPECT_CALL(_runtimeConfigMock, GetEventLatency(StrEq("testtenantid"), _)).
            WillRepeatedly(Return(EventLatency_Unspecified));
        EXPECT_CALL(_runtimeConfigMock, DecorateEvent(_, _, _)).
            WillRepeatedly(Invoke(&LoggerTests::fakeRuntimeConfigDecorateEvent));
        _logger.SetContext("contextvalue", "info");
        _sequenceId = 0;
    }

    void expectSubmit()
    {
        _submitted = false;
        EXPECT_CALL(_logger, submit(_, _, _,_)).
            WillOnce(DoAll(
            Assign(&_submitted, true),
            SaveArg<0>(&_submittedRecord),
            SaveArg<1>(&_submittedLatency))).
            RetiresOnSaturation();
    }

    void expectNoSubmit()
    {
        _submitted = false;
        EXPECT_CALL(_logger, submit(_, _, _,_)).
            Times(0);
    }

    void checkBaseAndContextAndRuntimeConfigProps(EventLatency latency = EventLatency_Normal)
    {
        UNREFERENCED_PARAMETER(latency);
        // Base
        EXPECT_THAT(_submittedRecord.name, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeinTicks();
        EXPECT_THAT(_submittedRecord.time, Gt(now - 60000000));
        EXPECT_THAT(_submittedRecord.time, Le(now));
        EXPECT_THAT(_submittedRecord.baseType, Not(IsEmpty()));
        //EXPECT_THAT(_submittedRecord.name["EventInfo.Name",       _submittedRecord.baseType)));
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.Source"].stringValue,"test-source");
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.Time"].stringValue,PAL::formatUtcTimestampMsAsISO8601(_submittedRecord.time));
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.InitId"].stringValue,   MatchesRegex(R"(........-....-....-....-............)"));
        //EXPECT_THAT(_submittedRecord.seqNum, ++_sequenceId);
        //EXPECT_THAT(_submittedRecord.ver, PAL::getSdkVersion());
        //EXPECT_THAT(_submittedRecord.ver, PAL::getSdkVersion());
        //EXPECT_THAT(_submittedRecord.data[0].properties["eventlatency"].longValue, latency);

        // Context
        EXPECT_THAT(_submittedRecord.data[0].properties["contextvalue"].stringValue, "info");

        // RuntimeConfig
        //EXPECT_THAT(_submittedRecord.data[0].properties["runtimeconfigvalue"].stringValue, "blah");
    }
};

TEST_F(LoggerTests, LogEvent)
{
    expectSubmit();
    _logger.LogEvent("name_only");
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq("name_only"));

    expectSubmit();
    EventProperties props1("name_only_props");
    _logger.LogEvent(props1);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq("name_only_props"));

    expectSubmit();
    EventProperties props2("custom_event");
    props2.SetProperty("test", "value");
    props2.SetProperty("auxiliary", "long content");
    props2.SetProperty("secret", "oops, I did it again", PiiKind_GenericData);
    _logger.LogEvent(props2);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq("custom_event"));
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");
    EXPECT_THAT(_submittedRecord.data[0].properties["auxiliary"].stringValue, "long content");
    ::AriaProtocol::PII pii;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    //pii.RawContent = "oops, I did it again";
    EXPECT_THAT(_submittedRecord.data[0].properties["secret"].stringValue, "oops, I did it again");
}

TEST_F(LoggerTests, CustomEventNameValidation)
{
	expectSubmit(); 
    EventProperties props("");
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

	expectSubmit();
    props.SetName(std::string(3, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName(std::string(4, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(100, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(101, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName("_" + std::string(99, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName(std::string(99, 'a') + "_");
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName("0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq("0123456789_abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz"));

    std::string banned("\x01" R"( !"#$%&'()*+,-./:;<=>?@[\]^`{|}~)" "\x81");
    EXPECT_CALL(_logger, submit(_, _, _, _)).WillRepeatedly(Assign(&_submitted, true));
    for (char ch : banned) {
        EventProperties props2(std::string("test") + ch + "char");
        _submitted = false;
        _logger.LogEvent(props2);
		if (ch == '.')
		{		
			EXPECT_THAT(_submitted, true) << "Banned character: '" << ch << "'";
		}
		else
		{
			EXPECT_THAT(_submitted, false) << "Banned character: '" << ch << "'";
		}
    }
}

TEST_F(LoggerTests, CustomPropertyNameValidation)
{
	expectSubmit();
    EventProperties props1("test");
    props1.SetProperty("", "x");
    _logger.LogEvent(props1);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props2("test");
    props2.SetProperty(std::string(1, 'a'), "x");
    _logger.LogEvent(props2);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props2.GetProperties())
	{
		EXPECT_THAT(_submittedRecord.data[0].properties[prop.first].stringValue, prop.second.to_string());
	}

    //EXPECT_THAT(_submittedRecord.data[0].properties, Contains(*props2.GetProperties().begin()));

    expectSubmit();
    EventProperties props3("test");
    props3.SetProperty(std::string(100, 'a'), "x");
    _logger.LogEvent(props3);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props3.GetProperties())
	{
		EXPECT_THAT(_submittedRecord.data[0].properties[prop.first].stringValue, prop.second.to_string());
	}
    //EXPECT_THAT(_submittedRecord.data[0].properties, Contains(*props3.GetProperties().begin()));

    expectSubmit();
    EventProperties props4("test");
    props4.SetProperty(std::string(101, 'a'), "x");
    _logger.LogEvent(props4);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props5("test");
    props5.SetProperty("_" + std::string(99, 'a'), "x");
    _logger.LogEvent(props5);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props6("test");
    props6.SetProperty(std::string(99, 'a') + "_", "x");
    _logger.LogEvent(props6);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props7("test");
    props7.SetProperty("." + std::string(99, 'a'), "x");
    _logger.LogEvent(props7);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props8("test");
    props8.SetProperty(std::string(99, 'a') + ".", "x");
    _logger.LogEvent(props8);
    ASSERT_THAT(_submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props9("test");
    props9.SetProperty("0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ.abcdefghijklmnopqrstuvwxyz", std::string(200, 'x'));
    _logger.LogEvent(props9);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props9.GetProperties())
	{
		EXPECT_THAT(_submittedRecord.data[0].properties[prop.first].stringValue, prop.second.to_string());
	}
    //EXPECT_THAT(_submittedRecord.data[0].properties, Contains(*props9.GetProperties().begin()));

#if ARIASDK_PAL_SKYPE
    // Sad but true. See comment in EventPropertiesDecorator::validatePropertyName().
    std::string banned("\x01" R"( !"#$%&'()*+,/;<=>?@[\]^`{|}~)" "\x81");
#else
    std::string banned("\x01" R"( !"#$%&'()*+,-/:;<=>?@[\]^`{|}~)" "\x81");
#endif
    EXPECT_CALL(_logger, submit(_, _, _, _)).WillRepeatedly(Assign(&_submitted, true));
    for (char ch : banned) {
        EventProperties props("test");
        props.SetProperty(std::string("test") + ch + "char", std::string(200, 'x'));
        _submitted = false;
        _logger.LogEvent(props);
        EXPECT_THAT(_submitted, true) << "Banned character: '" << ch << "'";
    }

    expectSubmit();
    EventProperties props10("test");
    props10.SetProperty("", "x", PiiKind_GenericData);
    _logger.LogEvent(props10);
    ASSERT_THAT(_submitted, true);
}

TEST_F(LoggerTests, CustomEventPropertiesCanOverrideOrEraseContextOnes)
{
    _logger.SetContext("plain1", "from-context");
    _logger.SetContext("plain2", "from-context");
    _logger.SetContext("pii1",   "from-context", PiiKind_GenericData);
    _logger.SetContext("pii2",   "from-context", PiiKind_GenericData);

    EventProperties props("overridden_event");
    props.SetProperty("plain1", "overridden");
    props.SetProperty("pii1",   "overridden", PiiKind_GenericData);
    props.SetProperty("plain2", "");
    props.SetProperty("pii2",   "", PiiKind_GenericData);
    expectSubmit();
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);

    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.name, Eq("overridden_event"));

    EXPECT_THAT(_submittedRecord.data[0].properties["plain1"].stringValue, "overridden");
    EXPECT_THAT(_submittedRecord.data[0].properties["plain2"].stringValue, "");

    ::AriaProtocol::PII pii;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    //pii.RawContent = "overridden";
    EXPECT_THAT(_submittedRecord.data[0].properties["pii1"].stringValue, "overridden");
    EXPECT_THAT(_submittedRecord.data[0].properties["pii2"].stringValue,"");
}



TEST_F(LoggerTests, RuntimeConfigLatencyIsForced)
{
    EventLatency forcedLatency = EventLatency_Normal;
    EXPECT_CALL(_runtimeConfigMock, GetEventLatency(StrEq("testtenantid"), StrEq("dummyname")))
        .WillRepeatedly(Return(forcedLatency));

    EventProperties eventProperties("dummyName");
    for (EventLatency latency : {EventLatency_Unspecified, EventLatency_Normal, EventLatency_RealTime, EventLatency_Max}) {
        eventProperties.SetLatency(latency);

		if (latency > EventLatency_Unspecified)
		{
            forcedLatency = latency;
		}

        expectSubmit();
        _logger.LogEvent(eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedLatency);
    }
}

TEST_F(LoggerTests, SubmitIgnoresLatencyOff)
{
    ::AriaProtocol::CsEvent record;
    record.baseType = "off";
	std::string name("test");
	std::uint64_t flags = 0;
    _logger.submit_(record, EventLatency_Off, EventPersistence_Normal, flags);
}

TEST_F(LoggerTests, SubmitSendsEventContext)
{
    ::AriaProtocol::CsEvent record;
    record.name = "guid";
    record.baseType = "eventtype";
    ::AriaProtocol::Value temp;
    temp.stringValue = "propertyvalue";
    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }
    record.data[0].properties["propertykey"] = temp;

    ARIASDK_NS::IncomingEventContextPtr event;
    EXPECT_CALL(_logManagerMock, addIncomingEvent(_))
        .WillOnce(SaveArg<0>(&event));
    _logger.submit_(record, EventLatency_Unspecified, EventPersistence_Normal,0);

    //EXPECT_THAT(event->record.id,          Eq("guid"));
    EXPECT_THAT(event->record.tenantToken, Eq("testtenantid-tenanttoken"));
    EXPECT_THAT(event->record.latency,     EventLatency_Unspecified);
    EXPECT_THAT(event->source->name,         Eq("guid"));
    EXPECT_THAT(event->source->baseType,  Eq("eventtype"));
    EXPECT_THAT(event->source->data[0].properties["propertykey"].stringValue,"propertyvalue");
}

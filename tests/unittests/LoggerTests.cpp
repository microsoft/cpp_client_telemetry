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

    MOCK_METHOD3(submit, void(::AriaProtocol::Record &, ::Microsoft::Applications::Telemetry::EventPriority, std::uint64_t  const& ));

    void submit_(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::EventPriority priority, std::uint64_t  const& policyBitFlags)
    {
        return Logger::submit(record, priority, policyBitFlags);
    }
};


class LoggerTests : public Test {
  protected:
    StrictMock<MockIRuntimeConfig>      _runtimeConfigMock;
    StrictMock<MockILogManagerInternal> _logManagerMock;
    Logger4Test                         _logger;

    EventProperties                     _emptyProperties;

    bool                                _submitted;
    ::AriaProtocol::Record              _submittedRecord;
    EventPriority                       _submittedPriority;

    uint64_t                            _sequenceId;

  protected:
    LoggerTests()
      : _logger("testtenantid-tenanttoken", "test-source", "ecs-project", _logManagerMock, nullptr, _runtimeConfigMock),
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
        EXPECT_CALL(_runtimeConfigMock, GetEventPriority(StrEq("testtenantid"), _)).
            WillRepeatedly(Return(EventPriority_Unspecified));
        EXPECT_CALL(_runtimeConfigMock, DecorateEvent(_, _, _)).
            WillRepeatedly(Invoke(&LoggerTests::fakeRuntimeConfigDecorateEvent));
        _logger.SetContext("contextvalue", "info");
        _sequenceId = 0;
    }

    void expectSubmit()
    {
        _submitted = false;
        EXPECT_CALL(_logger, submit(_, _, _)).
            WillOnce(DoAll(
            Assign(&_submitted, true),
            SaveArg<0>(&_submittedRecord),
            SaveArg<1>(&_submittedPriority))).
            RetiresOnSaturation();
    }

    void expectNoSubmit()
    {
        _submitted = false;
        EXPECT_CALL(_logger, submit(_, _, _)).
            Times(0);
    }

    void checkBaseAndContextAndRuntimeConfigProps(EventPriority priority = EventPriority_Normal)
    {
        // Base
        EXPECT_THAT(_submittedRecord.Id, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeMs();
        EXPECT_THAT(_submittedRecord.Timestamp, Gt(now - 60000));
        EXPECT_THAT(_submittedRecord.Timestamp, Le(now));
        EXPECT_THAT(_submittedRecord.Type, Eq("Custom"));
        EXPECT_THAT(_submittedRecord.RecordType, AriaProtocol::RecordType::Event);
        EXPECT_THAT(_submittedRecord.EventType, Not(IsEmpty()));
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("EventInfo.Name",       _submittedRecord.EventType)));
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("EventInfo.Source",     "test-source")));
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("EventInfo.Time",       PAL::formatUtcTimestampMsAsISO8601(_submittedRecord.Timestamp))));
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("EventInfo.InitId",     MatchesRegex(R"(........-....-....-....-............)"))));
        EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("EventInfo.Sequence",   ++_sequenceId)));
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("EventInfo.SdkVersion", PAL::getSdkVersion())));
        EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("eventpriority",        priority)));

        // Context
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("contextvalue", "info")));

        // RuntimeConfig
        EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("runtimeconfigvalue", "blah")));
    }
};


TEST_F(LoggerTests, LogAggregatedMetric)
{
    expectSubmit();
    _logger.LogAggregatedMetric("speed", 60000, 60, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Duration",      60000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Count",         60)));

    expectSubmit();
    AggregatedMetricData amd("speed", 60000, 60);
    amd.aggregates[AggregateType_Sum]          = 35 * 20 + 40 * 20 + 45 * 20;
    amd.aggregates[AggregateType_Maximum]      = 45;
    amd.aggregates[AggregateType_Minimum]      = 35;
    amd.aggregates[AggregateType_SumOfSquares] = 35 * 35 * 20 + 40 * 40 * 20 + 45 * 45 * 20;
    amd.buckets[0] = 0;
    amd.buckets[1] = 40;
    amd.buckets[2] = 20;
    amd.buckets[3] = 0;
    amd.instanceName = "Ferda";
    amd.objectClass  = "vehicle";
    amd.objectId     = "car-1";
    amd.units        = "km/h";
    _logger.LogAggregatedMetric(amd, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectClass",             "vehicle")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectId",                "car-1")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.InstanceName",            "Ferda")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Duration",       60000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Count",          60)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.Units",                   "km/h")));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("AggregatedMetric.Aggregates.Sum", 2400.000000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("AggregatedMetric.Aggregates.Maximum",45.000000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("AggregatedMetric.Aggregates.Minimum",35.000000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("AggregatedMetric.Aggregates.SumOfSquares", 97000.000000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Buckets.0",               0)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Buckets.1",               40)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Buckets.2",               20)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Buckets.3",               0)));

    expectSubmit();
    AggregatedMetricData amd2("speed", 60000, 60);
    amd2.instanceName = "Adref";
    amd2.objectClass  = "vehicle";
    amd2.objectId     = "car-2";
    amd2.units        = "km/h";
    EventProperties props("vehicle_stats");
    props.SetProperty("test", "value");
    _logger.LogAggregatedMetric(amd2, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectClass",             "vehicle")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectId",                "car-2")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.InstanceName",            "Adref")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Duration",       60000)));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("AggregatedMetric.Count",          60)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AggregatedMetric.Units",                   "km/h")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",                                     "value")));

    expectNoSubmit();
    _logger.LogAggregatedMetric("", 60000, 60, _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogAppLifecycle)
{
    expectSubmit();
    _logger.LogAppLifecycle(AppLifecycleState_Launch, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("AppLifecycle"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AppLifeCycle.State", "Launch")));

    expectSubmit();
    EventProperties props("ui_process_state");
    props.SetProperty("test", "value");
    _logger.LogAppLifecycle(AppLifecycleState_Exit, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("AppLifecycle"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("AppLifeCycle.State", "Exit")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",               "value")));
}

TEST_F(LoggerTests, LogEvent)
{
    expectSubmit();
    _logger.LogEvent("name_only");
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("name_only"));

    expectSubmit();
    EventProperties props1("name_only_props");
    _logger.LogEvent(props1);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("name_only_props"));

    expectSubmit();
    EventProperties props2("custom_event");
    props2.SetProperty("test", "value");
    props2.SetProperty("auxiliary", "long content");
    props2.SetProperty("secret", "oops, I did it again", PiiKind_GenericData);
    _logger.LogEvent(props2);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("custom_event"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",      "value")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("auxiliary", "long content")));
    ::AriaProtocol::PII pii;
    pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    pii.RawContent = "oops, I did it again";
    EXPECT_THAT(_submittedRecord.PIIExtensions, Contains(Pair("secret", pii)));
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
    EXPECT_THAT(_submittedRecord.EventType, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(100, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq(props.GetName()));

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
    EXPECT_THAT(_submittedRecord.EventType, Eq("0123456789_abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz"));

    std::string banned("\x01" R"( !"#$%&'()*+,-./:;<=>?@[\]^`{|}~)" "\x81");
    EXPECT_CALL(_logger, submit(_, _, _)).WillRepeatedly(Assign(&_submitted, true));
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
		EXPECT_THAT(_submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}

  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props2.GetProperties..GetProperties().begin()));

    expectSubmit();
    EventProperties props3("test");
    props3.SetProperty(std::string(100, 'a'), "x");
    _logger.LogEvent(props3);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props3.GetProperties())
	{
		EXPECT_THAT(_submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}
  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props3.GetProperties().begin()));

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
		EXPECT_THAT(_submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}
  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props9.GetProperties().begin()));

#if ARIASDK_PAL_SKYPE
    // Sad but true. See comment in EventPropertiesDecorator::validatePropertyName().
    std::string banned("\x01" R"( !"#$%&'()*+,/;<=>?@[\]^`{|}~)" "\x81");
#else
    std::string banned("\x01" R"( !"#$%&'()*+,-/:;<=>?@[\]^`{|}~)" "\x81");
#endif
    EXPECT_CALL(_logger, submit(_, _, _)).WillRepeatedly(Assign(&_submitted, true));
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
    EXPECT_THAT(_submittedRecord.EventType, Eq("overridden_event"));

 //   EXPECT_THAT(submittedRecord.Extension, Contains(Pair("plain1", "overridden")));
 //   EXPECT_THAT(submittedRecord.Extension, Not(Contains(Pair("plain2", _))));

    ::AriaProtocol::PII pii;
    pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    pii.RawContent = "overridden";
 //   EXPECT_THAT(submittedRecord.PIIExtensions, Contains(Pair("pii1", pii)));
 //   EXPECT_THAT(submittedRecord.PIIExtensions, Not(Contains(Pair("pii2", _))));
}

TEST_F(LoggerTests, LogFailure)
{
    expectSubmit();
    _logger.LogFailure("bad problem", "no food", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("Failure"));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Signature", "bad problem")));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Detail",    "no food")));

    expectSubmit();
    _logger.LogFailure("worse problem", "no water", "serious", "#555", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("Failure"));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Signature", "worse problem")));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Detail",    "no water")));
 //   EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Category",  "serious")));
 //   EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Id",        "#555")));

    expectSubmit();
    EventProperties props("problem_report");
    props.SetProperty("life_support_eta", "7:30");
    _logger.LogFailure("catastrophic problem", "oxygen tank exploded", "deadly", "#666", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("Failure"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Failure.Signature", "catastrophic problem")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Failure.Detail",    "oxygen tank exploded")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Failure.Category",  "deadly")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Failure.Id",        "#666")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("life_support_eta",  "7:30")));

    expectNoSubmit();
    _logger.LogFailure("empty detail", "", _emptyProperties);
    ASSERT_THAT(_submitted, false);

    expectNoSubmit();
    _logger.LogFailure("", "empty signature", _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogPageAction)
{
    expectSubmit();
    _logger.LogPageAction("/index.html", ActionType_Click, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Click")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "/index.html")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "Unspecified")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Unspecified")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("PageAction.TargetItemLayout.Rank", 0)));

    expectSubmit();
    PageActionData pad("/eshop/cart.aspx", ActionType_Zoom);
    pad.inputDeviceType = InputDeviceType_Touch;
    pad.rawActionType   = RawActionType_TouchZoom;
    _logger.LogPageAction(pad, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Zoom")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "/eshop/cart.aspx")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "TouchZoom")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Touch")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("PageAction.TargetItemLayout.Rank", 0)));

    expectSubmit();
    PageActionData pad2("SendMessage.action", ActionType_Other);
    pad2.destinationUri = "SendAttachment.action";
    pad2.inputDeviceType = InputDeviceType_Keyboard;
    pad2.rawActionType   = RawActionType_KeyboardPress;
    pad2.targetItemDataSourceCategory   = "form";
    pad2.targetItemDataSourceCollection = "buttons";
    pad2.targetItemDataSourceName       = "extras";
    pad2.targetItemId                   = "attach";
    pad2.targetItemLayoutContainer      = "message-entry";
    pad2.targetItemLayoutRank           = 123;
    EventProperties props("page_change_report");
    props.SetProperty("test", "value");
    _logger.LogPageAction(pad2, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Other")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "SendMessage.action")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "KeyboardPress")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Keyboard")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.DestinationUri",                  "SendAttachment.action")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.TargetItemId",                    "attach")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Name",       "extras")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Category",   "form")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Collection", "buttons")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageAction.TargetItemLayout.Container",      "message-entry")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("PageAction.TargetItemLayout.Rank", 123)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",                                       "value")));

    expectNoSubmit();
    _logger.LogPageAction("", ActionType_Click, _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogPageView)
{
    expectSubmit();
    _logger.LogPageView("id-1", "Index", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-1")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Name",        "Index")));

    expectSubmit();
    _logger.LogPageView("id-2", "Cart", "eshop", "/eshop/cart.aspx", "/eshop/index.aspx", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-2")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Name",        "Cart")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Category",    "eshop")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Uri",         "/eshop/cart.aspx")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.ReferrerUri", "/eshop/index.aspx")));

    expectSubmit();
    EventProperties props("internal_browser_page_view");
    props.SetProperty("test", "value");
    _logger.LogPageView("id-3", "Order", "eshop", "/eshop/order.aspx", "/eshop/cart.aspx", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-3")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Name",        "Order")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Category",    "eshop")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.Uri",         "/eshop/order.aspx")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("PageView.ReferrerUri", "/eshop/cart.aspx")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",                 "value")));

    expectNoSubmit();
    _logger.LogPageView("", "name", _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogSampledMetric)
{
    expectSubmit();
    _logger.LogSampledMetric("measurement", 1.2, "ms", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "measurement")));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("SampledMetric.Value",1.200000)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "ms")));

    expectSubmit();
    _logger.LogSampledMetric("speed", 67.89, "km/h", "Ferda", "vehicle", "id-1", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "speed")));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("SampledMetric.Value",        67.890000)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "km/h")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.InstanceName", "Ferda")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectClass",  "vehicle")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectId",     "id-1")));

    expectSubmit();
    EventProperties props("detected_vehicle");
    _logger.LogSampledMetric("speed", 89.01, "km/h", "Adref", "vehicle", "id-2", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "speed")));
    EXPECT_THAT(_submittedRecord.TypedExtensionDouble, Contains(Pair("SampledMetric.Value",        89.010000)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "km/h")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.InstanceName", "Adref")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectClass",  "vehicle")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectId",     "id-2")));

    expectNoSubmit();
    _logger.LogSampledMetric("", 1.2, "ms", _emptyProperties);
    ASSERT_THAT(_submitted, false);

    expectNoSubmit();
    _logger.LogSampledMetric("measurement", 1.2, "", _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogTrace)
{
    expectSubmit();
    _logger.LogTrace(TraceLevel_Information, "Everything is all right", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("Trace"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Trace.Level",   "Information")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Trace.Message", "Everything is all right")));

    expectSubmit();
    EventProperties props("worker_report");
    props.SetProperty("test", "value");
    _logger.LogTrace(TraceLevel_Warning, "Stuff is getting harder", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("Trace"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Trace.Level",   "Warning")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("Trace.Message", "Stuff is getting harder")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",          "value")));

    expectNoSubmit();
    _logger.LogTrace(TraceLevel_Warning, "", _emptyProperties);
    ASSERT_THAT(_submitted, false);
}

TEST_F(LoggerTests, LogUserState)
{
    expectSubmit();
    _logger.LogUserState(UserState_Connected, 12345, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("UserInfo_UserState"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("State.Name",         "UserState")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("State.Value",        "Connected")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("State.TimeToLive",   12345)));
    EXPECT_THAT(_submittedRecord.TypedExtensionBoolean, Contains(Pair("State.IsTransition", true)));

    expectSubmit();
    EventProperties props("app_login_state");
    props.SetProperty("test", "value");
    _logger.LogUserState(UserState_SignedIn, 67890, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.EventType, Eq("UserInfo_UserState"));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("State.Name",         "UserState")));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("State.Value",        "SignedIn")));
    EXPECT_THAT(_submittedRecord.TypedExtensionInt64, Contains(Pair("State.TimeToLive",   67890)));
    EXPECT_THAT(_submittedRecord.TypedExtensionBoolean, Contains(Pair("State.IsTransition", true)));
    EXPECT_THAT(_submittedRecord.Extension, Contains(Pair("test",               "value")));
}

TEST_F(LoggerTests, RuntimeConfigPriorityIsForced)
{
    EventPriority forcedPriority = EventPriority_Normal;
    EXPECT_CALL(_runtimeConfigMock, GetEventPriority(StrEq("testtenantid"), StrEq("dummyname")))
        .WillRepeatedly(Return(forcedPriority));

    EventProperties eventProperties("dummyName");
    for (EventPriority priority : {EventPriority_Unspecified, EventPriority_Low, EventPriority_Normal, EventPriority_High, EventPriority_Immediate}) {
        eventProperties.SetPriority(priority);

		if (priority > EventPriority_Unspecified)
		{
			forcedPriority = priority;
		}

        expectSubmit();
        _logger.LogAggregatedMetric("speed", 60000, 60, eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogAppLifecycle(AppLifecycleState_Launch, eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogEvent(eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogFailure("bad problem", "no food", eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogPageAction("/index.html", ActionType_Click, eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogPageView("id-1", "Index", eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogSampledMetric("measurement", 1.2, "ms", eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogTrace(TraceLevel_Information, "Everything is all right", eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        _logger.LogUserState(UserState_Connected, 12345, eventProperties);
        EXPECT_THAT(_submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);
    }
}

TEST_F(LoggerTests, SubmitIgnoresPriorityOff)
{
    ::AriaProtocol::Record record;
    record.EventType = "off";
	std::string name("test");
	std::uint64_t flags = 0;
    _logger.submit_(record, EventPriority_Off, flags);
}

TEST_F(LoggerTests, SubmitSendsEventContext)
{
    ::AriaProtocol::Record record;
    record.Id = "guid";
    record.EventType = "eventtype";
    record.Extension["propertykey"] = "propertyvalue";

    ARIASDK_NS::IncomingEventContextPtr event;
    EXPECT_CALL(_logManagerMock, addIncomingEvent(_))
        .WillOnce(SaveArg<0>(&event));
    _logger.submit_(record, EventPriority_Unspecified,0);

    EXPECT_THAT(event->record.id,          Eq("guid"));
    EXPECT_THAT(event->record.tenantToken, Eq("testtenantid-tenanttoken"));
    EXPECT_THAT(event->record.priority,    EventPriority_Unspecified);
    EXPECT_THAT(event->source->Id,         Eq("guid"));
    EXPECT_THAT(event->source->EventType,  Eq("eventtype"));
    EXPECT_THAT(event->source->Extension,  Contains(Pair(Eq("propertykey"), Eq("propertyvalue"))));
}

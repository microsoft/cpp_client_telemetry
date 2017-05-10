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

    MOCK_METHOD2(submit, void(::AriaProtocol::Record &, ::Microsoft::Applications::Telemetry::EventPriority));

    void submit_(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::EventPriority priority)
    {
        return Logger::submit(record, priority);
    }
};


class LoggerTests : public Test {
  protected:
    StrictMock<MockIRuntimeConfig>      runtimeConfigMock;
    StrictMock<MockILogManagerInternal> logManagerMock;
    Logger4Test                         logger;

    EventProperties                     emptyProperties;

    bool                                submitted;
    ::AriaProtocol::Record              submittedRecord;
    EventPriority                       submittedPriority;

    uint64_t                            sequenceId;

  protected:
    LoggerTests()
      : logger("testtenantid-tenanttoken", "test-source", "ecs-project", logManagerMock, nullptr, runtimeConfigMock),
        emptyProperties("")
    {
    }

    static void fakeRuntimeConfigDecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName)
    {
        extension["runtimeconfigvalue"] = "blah";
    }

    virtual void SetUp() override
    {
        EXPECT_CALL(runtimeConfigMock, GetEventPriority(StrEq("testtenantid"), _)).
            WillRepeatedly(Return(EventPriority_Unspecified));
        EXPECT_CALL(runtimeConfigMock, DecorateEvent(_, _, _)).
            WillRepeatedly(Invoke(&LoggerTests::fakeRuntimeConfigDecorateEvent));
        logger.SetContext("contextvalue", "info");
        sequenceId = 0;
    }

    void expectSubmit()
    {
        submitted = false;
        EXPECT_CALL(logger, submit(_, _)).
            WillOnce(DoAll(
            Assign(&submitted, true),
            SaveArg<0>(&submittedRecord),
            SaveArg<1>(&submittedPriority))).
            RetiresOnSaturation();
    }

    void expectNoSubmit()
    {
        submitted = false;
        EXPECT_CALL(logger, submit(_, _)).
            Times(0);
    }

    void checkBaseAndContextAndRuntimeConfigProps(EventPriority priority = EventPriority_Normal)
    {
        // Base
        EXPECT_THAT(submittedRecord.Id, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeMs();
        EXPECT_THAT(submittedRecord.Timestamp, Gt(now - 60000));
        EXPECT_THAT(submittedRecord.Timestamp, Le(now));
        EXPECT_THAT(submittedRecord.Type, Eq("Custom"));
        EXPECT_THAT(submittedRecord.RecordType, AriaProtocol::RecordType::Event);
        EXPECT_THAT(submittedRecord.EventType, Not(IsEmpty()));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.Name",       submittedRecord.EventType)));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.Source",     "test-source")));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.Time",       PAL::formatUtcTimestampMsAsISO8601(submittedRecord.Timestamp))));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.InitId",     MatchesRegex(R"(........-....-....-....-............)"))));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.Sequence",   toString(++sequenceId))));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("EventInfo.SdkVersion", PAL::getSdkVersion())));
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("eventpriority",        toString(priority))));

        // Context
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("contextvalue", "info")));

        // RuntimeConfig
        EXPECT_THAT(submittedRecord.Extension, Contains(Pair("runtimeconfigvalue", "blah")));
    }
};


TEST_F(LoggerTests, LogAggregatedMetric)
{
    expectSubmit();
    logger.LogAggregatedMetric("speed", 60000, 60, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Duration",                "60000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Count",                   "60")));

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
    logger.LogAggregatedMetric(amd, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectClass",             "vehicle")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectId",                "car-1")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.InstanceName",            "Ferda")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Duration",                "60000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Count",                   "60")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Units",                   "km/h")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Aggregates.Sum",          "2400.000000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Aggregates.Maximum",      "45.000000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Aggregates.Minimum",      "35.000000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Aggregates.SumOfSquares", "97000.000000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Buckets.0",               "0")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Buckets.1",               "40")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Buckets.2",               "20")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Buckets.3",               "0")));

    expectSubmit();
    AggregatedMetricData amd2("speed", 60000, 60);
    amd2.instanceName = "Adref";
    amd2.objectClass  = "vehicle";
    amd2.objectId     = "car-2";
    amd2.units        = "km/h";
    EventProperties props("vehicle_stats");
    props.SetProperty("test", "value");
    logger.LogAggregatedMetric(amd2, props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("AggregatedMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectClass",             "vehicle")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.ObjectId",                "car-2")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Name",                    "speed")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.InstanceName",            "Adref")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Duration",                "60000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Count",                   "60")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AggregatedMetric.Units",                   "km/h")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",                                     "value")));

    expectNoSubmit();
    logger.LogAggregatedMetric("", 60000, 60, emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogAppLifecycle)
{
    expectSubmit();
    logger.LogAppLifecycle(AppLifecycleState_Launch, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("AppLifecycle"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AppLifeCycle.State", "Launch")));

    expectSubmit();
    EventProperties props("ui_process_state");
    props.SetProperty("test", "value");
    logger.LogAppLifecycle(AppLifecycleState_Exit, props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("AppLifecycle"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("AppLifeCycle.State", "Exit")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",               "value")));
}

TEST_F(LoggerTests, LogEvent)
{
    expectSubmit();
    logger.LogEvent("name_only");
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("name_only"));

    expectSubmit();
    EventProperties props1("name_only_props");
    logger.LogEvent(props1);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("name_only_props"));

    expectSubmit();
    EventProperties props2("custom_event");
    props2.SetProperty("test", "value");
    props2.SetProperty("auxiliary", "long content");
    props2.SetProperty("secret", "oops, I did it again", PiiKind_GenericData);
    logger.LogEvent(props2);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("custom_event"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",      "value")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("auxiliary", "long content")));
    ::AriaProtocol::PII pii;
    pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    pii.RawContent = "oops, I did it again";
    EXPECT_THAT(submittedRecord.PIIExtensions, Contains(Pair("secret", pii)));
}

TEST_F(LoggerTests, CustomEventNameValidation)
{
	expectSubmit(); 
    EventProperties props("");
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

	expectSubmit();
    props.SetName(std::string(3, 'a'));
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName(std::string(4, 'a'));
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(100, 'a'));
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(101, 'a'));
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName("_" + std::string(99, 'a'));
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName(std::string(99, 'a') + "_");
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    props.SetName("0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("0123456789_abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz"));

    std::string banned("\x01" R"( !"#$%&'()*+,-./:;<=>?@[\]^`{|}~)" "\x81");
    EXPECT_CALL(logger, submit(_, _)).WillRepeatedly(Assign(&submitted, true));
    for (char ch : banned) {
        EventProperties props(std::string("test") + ch + "char");
        submitted = false;
        logger.LogEvent(props);
		if (ch == '.')
		{		
			EXPECT_THAT(submitted, true) << "Banned character: '" << ch << "'";
		}
		else
		{
			EXPECT_THAT(submitted, false) << "Banned character: '" << ch << "'";
		}
    }
}

TEST_F(LoggerTests, CustomPropertyNameValidation)
{
	expectSubmit();
    EventProperties props1("test");
    props1.SetProperty("", "x");
    logger.LogEvent(props1);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props2("test");
    props2.SetProperty(std::string(1, 'a'), "x");
    logger.LogEvent(props2);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props2.GetProperties())
	{
		EXPECT_THAT(submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}

  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props2.GetProperties..GetProperties().begin()));

    expectSubmit();
    EventProperties props3("test");
    props3.SetProperty(std::string(100, 'a'), "x");
    logger.LogEvent(props3);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props3.GetProperties())
	{
		EXPECT_THAT(submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}
  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props3.GetProperties().begin()));

    expectSubmit();
    EventProperties props4("test");
    props4.SetProperty(std::string(101, 'a'), "x");
    logger.LogEvent(props4);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props5("test");
    props5.SetProperty("_" + std::string(99, 'a'), "x");
    logger.LogEvent(props5);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props6("test");
    props6.SetProperty(std::string(99, 'a') + "_", "x");
    logger.LogEvent(props6);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props7("test");
    props7.SetProperty("." + std::string(99, 'a'), "x");
    logger.LogEvent(props7);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props8("test");
    props8.SetProperty(std::string(99, 'a') + ".", "x");
    logger.LogEvent(props8);
    ASSERT_THAT(submitted, true);
	checkBaseAndContextAndRuntimeConfigProps();

    expectSubmit();
    EventProperties props9("test");
    props9.SetProperty("0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ.abcdefghijklmnopqrstuvwxyz", std::string(200, 'x'));
    logger.LogEvent(props9);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
	for (std::pair<std::string, EventProperty> prop : props9.GetProperties())
	{
		EXPECT_THAT(submittedRecord.Extension, Contains(Pair(prop.first, prop.second.to_string())));
	}
  //  EXPECT_THAT(submittedRecord.Extension, Contains(*props9.GetProperties().begin()));

#if ARIASDK_PAL_SKYPE
    // Sad but true. See comment in EventPropertiesDecorator::validatePropertyName().
    std::string banned("\x01" R"( !"#$%&'()*+,/;<=>?@[\]^`{|}~)" "\x81");
#else
    std::string banned("\x01" R"( !"#$%&'()*+,-/:;<=>?@[\]^`{|}~)" "\x81");
#endif
    EXPECT_CALL(logger, submit(_, _)).WillRepeatedly(Assign(&submitted, true));
    for (char ch : banned) {
        EventProperties props("test");
        props.SetProperty(std::string("test") + ch + "char", std::string(200, 'x'));
        submitted = false;
        logger.LogEvent(props);
        EXPECT_THAT(submitted, true) << "Banned character: '" << ch << "'";
    }

    expectSubmit();
    EventProperties props10("test");
    props10.SetProperty("", "x", PiiKind_GenericData);
    logger.LogEvent(props10);
    ASSERT_THAT(submitted, true);
}

TEST_F(LoggerTests, CustomEventPropertiesCanOverrideOrEraseContextOnes)
{
    logger.SetContext("plain1", "from-context");
    logger.SetContext("plain2", "from-context");
    logger.SetContext("pii1",   "from-context", PiiKind_GenericData);
    logger.SetContext("pii2",   "from-context", PiiKind_GenericData);

    EventProperties props("overridden_event");
    props.SetProperty("plain1", "overridden");
    props.SetProperty("pii1",   "overridden", PiiKind_GenericData);
    props.SetProperty("plain2", "");
    props.SetProperty("pii2",   "", PiiKind_GenericData);
    expectSubmit();
    logger.LogEvent(props);
    ASSERT_THAT(submitted, true);

    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("overridden_event"));

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
    logger.LogFailure("bad problem", "no food", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("Failure"));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Signature", "bad problem")));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Detail",    "no food")));

    expectSubmit();
    logger.LogFailure("worse problem", "no water", "serious", "#555", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("Failure"));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Signature", "worse problem")));
  //  EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Detail",    "no water")));
 //   EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Category",  "serious")));
 //   EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Id",        "#555")));

    expectSubmit();
    EventProperties props("problem_report");
    props.SetProperty("life_support_eta", "7:30");
    logger.LogFailure("catastrophic problem", "oxygen tank exploded", "deadly", "#666", props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("Failure"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Signature", "catastrophic problem")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Detail",    "oxygen tank exploded")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Category",  "deadly")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Failure.Id",        "#666")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("life_support_eta",  "7:30")));

    expectNoSubmit();
    logger.LogFailure("empty detail", "", emptyProperties);
    ASSERT_THAT(submitted, false);

    expectNoSubmit();
    logger.LogFailure("", "empty signature", emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogPageAction)
{
    expectSubmit();
    logger.LogPageAction("/index.html", ActionType_Click, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Click")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "/index.html")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "Unspecified")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Unspecified")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemLayout.Rank",           "0")));

    expectSubmit();
    PageActionData pad("/eshop/cart.aspx", ActionType_Zoom);
    pad.inputDeviceType = InputDeviceType_Touch;
    pad.rawActionType   = RawActionType_TouchZoom;
    logger.LogPageAction(pad, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Zoom")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "/eshop/cart.aspx")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "TouchZoom")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Touch")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemLayout.Rank",           "0")));

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
    logger.LogPageAction(pad2, props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageAction"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.ActionType",                      "Other")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.PageViewId",                      "SendMessage.action")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.RawActionType",                   "KeyboardPress")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.InputDeviceType",                 "Keyboard")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.DestinationUri",                  "SendAttachment.action")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemId",                    "attach")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Name",       "extras")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Category",   "form")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemDataSource.Collection", "buttons")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemLayout.Container",      "message-entry")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageAction.TargetItemLayout.Rank",           "123")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",                                       "value")));

    expectNoSubmit();
    logger.LogPageAction("", ActionType_Click, emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogPageView)
{
    expectSubmit();
    logger.LogPageView("id-1", "Index", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-1")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Name",        "Index")));

    expectSubmit();
    logger.LogPageView("id-2", "Cart", "eshop", "/eshop/cart.aspx", "/eshop/index.aspx", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-2")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Name",        "Cart")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Category",    "eshop")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Uri",         "/eshop/cart.aspx")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.ReferrerUri", "/eshop/index.aspx")));

    expectSubmit();
    EventProperties props("internal_browser_page_view");
    props.SetProperty("test", "value");
    logger.LogPageView("id-3", "Order", "eshop", "/eshop/order.aspx", "/eshop/cart.aspx", props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("PageView"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Id",          "id-3")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Name",        "Order")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Category",    "eshop")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.Uri",         "/eshop/order.aspx")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("PageView.ReferrerUri", "/eshop/cart.aspx")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",                 "value")));

    expectNoSubmit();
    logger.LogPageView("", "name", emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogSampledMetric)
{
    expectSubmit();
    logger.LogSampledMetric("measurement", 1.2, "ms", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "measurement")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Value",        "1.200000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "ms")));

    expectSubmit();
    logger.LogSampledMetric("speed", 67.89, "km/h", "Ferda", "vehicle", "id-1", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "speed")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Value",        "67.890000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "km/h")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.InstanceName", "Ferda")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectClass",  "vehicle")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectId",     "id-1")));

    expectSubmit();
    EventProperties props("detected_vehicle");
    logger.LogSampledMetric("speed", 89.01, "km/h", "Adref", "vehicle", "id-2", props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("SampledMetric"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Name",         "speed")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Value",        "89.010000")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.Units",        "km/h")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.InstanceName", "Adref")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectClass",  "vehicle")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("SampledMetric.ObjectId",     "id-2")));

    expectNoSubmit();
    logger.LogSampledMetric("", 1.2, "ms", emptyProperties);
    ASSERT_THAT(submitted, false);

    expectNoSubmit();
    logger.LogSampledMetric("measurement", 1.2, "", emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogTrace)
{
    expectSubmit();
    logger.LogTrace(TraceLevel_Information, "Everything is all right", emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("Trace"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Trace.Level",   "Information")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Trace.Message", "Everything is all right")));

    expectSubmit();
    EventProperties props("worker_report");
    props.SetProperty("test", "value");
    logger.LogTrace(TraceLevel_Warning, "Stuff is getting harder", props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("Trace"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Trace.Level",   "Warning")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("Trace.Message", "Stuff is getting harder")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",          "value")));

    expectNoSubmit();
    logger.LogTrace(TraceLevel_Warning, "", emptyProperties);
    ASSERT_THAT(submitted, false);
}

TEST_F(LoggerTests, LogUserState)
{
    expectSubmit();
    logger.LogUserState(UserState_Connected, 12345, emptyProperties);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("UserInfo_UserState"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.Name",         "UserState")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.Value",        "Connected")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.TimeToLive",   "12345")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.IsTransition", "true")));

    expectSubmit();
    EventProperties props("app_login_state");
    props.SetProperty("test", "value");
    logger.LogUserState(UserState_SignedIn, 67890, props);
    ASSERT_THAT(submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(submittedRecord.EventType, Eq("UserInfo_UserState"));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.Name",         "UserState")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.Value",        "SignedIn")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.TimeToLive",   "67890")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("State.IsTransition", "true")));
    EXPECT_THAT(submittedRecord.Extension, Contains(Pair("test",               "value")));
}

TEST_F(LoggerTests, RuntimeConfigPriorityIsForced)
{
    EventPriority forcedPriority = EventPriority_Normal;
    EXPECT_CALL(runtimeConfigMock, GetEventPriority(StrEq("testtenantid"), StrEq("dummyname")))
        .WillRepeatedly(Return(forcedPriority));

    EventProperties eventProperties("dummyName");
    for (EventPriority priority : {EventPriority_Unspecified, EventPriority_Low, EventPriority_Normal, EventPriority_High, EventPriority_Immediate}) {
        eventProperties.SetPriority(priority);

		if (priority > EventPriority_Unspecified)
		{
			forcedPriority = priority;
		}

        expectSubmit();
        logger.LogAggregatedMetric("speed", 60000, 60, eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogAppLifecycle(AppLifecycleState_Launch, eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogEvent(eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogFailure("bad problem", "no food", eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogPageAction("/index.html", ActionType_Click, eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogPageView("id-1", "Index", eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogSampledMetric("measurement", 1.2, "ms", eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogTrace(TraceLevel_Information, "Everything is all right", eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);

        expectSubmit();
        logger.LogUserState(UserState_Connected, 12345, eventProperties);
        EXPECT_THAT(submitted, true);
        checkBaseAndContextAndRuntimeConfigProps(forcedPriority);
    }
}

TEST_F(LoggerTests, SubmitIgnoresPriorityOff)
{
    ::AriaProtocol::Record record;
    record.EventType = "off";
    logger.submit_(record, EventPriority_Off);
}

TEST_F(LoggerTests, SubmitSendsEventContext)
{
    ::AriaProtocol::Record record;
    record.Id = "guid";
    record.EventType = "eventtype";
    record.Extension["propertykey"] = "propertyvalue";

    ARIASDK_NS::IncomingEventContextPtr event;
    EXPECT_CALL(logManagerMock, addIncomingEvent(_))
        .WillOnce(SaveArg<0>(&event));
    logger.submit_(record, EventPriority_Unspecified);

    EXPECT_THAT(event->record.id,          Eq("guid"));
    EXPECT_THAT(event->record.tenantToken, Eq("testtenantid-tenanttoken"));
    EXPECT_THAT(event->record.priority,    EventPriority_Unspecified);
    EXPECT_THAT(event->source->Id,         Eq("guid"));
    EXPECT_THAT(event->source->EventType,  Eq("eventtype"));
    EXPECT_THAT(event->source->Extension,  Contains(Pair(Eq("propertykey"), Eq("propertyvalue"))));
}

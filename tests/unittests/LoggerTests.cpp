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

    MOCK_METHOD3(submit, void(::AriaProtocol::CsEvent &, ::Microsoft::Applications::Telemetry::EventPriority, std::uint64_t  const& ));

    void submit_(::AriaProtocol::CsEvent& record, ::Microsoft::Applications::Telemetry::EventPriority priority, std::uint64_t  const& policyBitFlags)
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
    ::AriaProtocol::CsEvent              _submittedRecord;
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
        UNREFERENCED_PARAMETER(priority);
        // Base
        EXPECT_THAT(_submittedRecord.name, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeMs();
        EXPECT_THAT(_submittedRecord.time, Gt(now - 60000));
        EXPECT_THAT(_submittedRecord.time, Le(now));
        EXPECT_THAT(_submittedRecord.baseType, Not(IsEmpty()));
        //EXPECT_THAT(_submittedRecord.name["EventInfo.Name",       _submittedRecord.baseType)));
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.Source"].stringValue,"test-source");
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.Time"].stringValue,PAL::formatUtcTimestampMsAsISO8601(_submittedRecord.time));
        //EXPECT_THAT(_submittedRecord.data[0].properties["EventInfo.InitId"].stringValue,   MatchesRegex(R"(........-....-....-....-............)"));
        EXPECT_THAT(_submittedRecord.seqNum, ++_sequenceId);
        EXPECT_THAT(_submittedRecord.ver, PAL::getSdkVersion());
        //EXPECT_THAT(_submittedRecord.data[0].properties["eventpriority"].longValue, priority);

        // Context
        EXPECT_THAT(_submittedRecord.data[0].properties["contextvalue"].stringValue, "info");

        // RuntimeConfig
        //EXPECT_THAT(_submittedRecord.data[0].properties["runtimeconfigvalue"].stringValue, "blah");
    }
};


TEST_F(LoggerTests, LogAggregatedMetric)
{
    expectSubmit();
    _logger.LogAggregatedMetric("speed", 60000, 60, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Name"].stringValue, "speed");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Duration"].longValue, 60000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Count"].longValue, 60);

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.ObjectClass"].stringValue,"vehicle");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.ObjectId"].stringValue, "car-1");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Name"].stringValue, "speed");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.InstanceName"].stringValue, "Ferda");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Duration"].longValue, 60000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Count"].longValue, 60);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Units"].stringValue, "km/h");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Aggregates.Sum"].doubleValue, 2400.000000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Aggregates.Maximum"].doubleValue, 45.000000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Aggregates.Minimum"].doubleValue, 35.000000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Aggregates.SumOfSquares"].doubleValue, 97000.000000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Buckets.0"].longValue, 0);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Buckets.1"].longValue, 40);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Buckets.2"].longValue, 20);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Buckets.3"].longValue, 0);

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("AggregatedMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.ObjectClass"].stringValue, "vehicle");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.ObjectId"].stringValue, "car-2");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Name"].stringValue, "speed");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.InstanceName"].stringValue, "Adref");
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Duration"].longValue, 60000);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Count"].longValue,  60);
    EXPECT_THAT(_submittedRecord.data[0].properties["AggregatedMetric.Units"].stringValue, "km/h");
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("AppLifecycle"));
    EXPECT_THAT(_submittedRecord.data[0].properties["AppLifeCycle.State"].stringValue, "Launch");

    expectSubmit();
    EventProperties props("ui_process_state");
    props.SetProperty("test", "value");
    _logger.LogAppLifecycle(AppLifecycleState_Exit, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("AppLifecycle"));
    EXPECT_THAT(_submittedRecord.data[0].properties["AppLifeCycle.State"].stringValue, "Exit");
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");
}

TEST_F(LoggerTests, LogEvent)
{
    expectSubmit();
    _logger.LogEvent("name_only");
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("name_only"));

    expectSubmit();
    EventProperties props1("name_only_props");
    _logger.LogEvent(props1);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("name_only_props"));

    expectSubmit();
    EventProperties props2("custom_event");
    props2.SetProperty("test", "value");
    props2.SetProperty("auxiliary", "long content");
    props2.SetProperty("secret", "oops, I did it again", PiiKind_GenericData);
    _logger.LogEvent(props2);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("custom_event"));
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
    EXPECT_THAT(_submittedRecord.baseType, Eq(props.GetName()));

    expectSubmit();
    props.SetName(std::string(100, 'a'));
    _logger.LogEvent(props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq(props.GetName()));

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("0123456789_abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz"));

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("overridden_event"));

    EXPECT_THAT(_submittedRecord.data[0].properties["plain1"].stringValue, "overridden");
    EXPECT_THAT(_submittedRecord.data[0].properties["plain2"].stringValue, "");

    ::AriaProtocol::PII pii;
    pii.Kind       = ::AriaProtocol::PIIKind::GenericData;
    //pii.RawContent = "overridden";
    EXPECT_THAT(_submittedRecord.data[0].properties["pii1"].stringValue, "overridden");
    EXPECT_THAT(_submittedRecord.data[0].properties["pii2"].stringValue,"");
}

TEST_F(LoggerTests, LogFailure)
{
    expectSubmit();
    _logger.LogFailure("bad problem", "no food", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("Failure"));
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Signature"].stringValue, "bad problem");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Detail"].stringValue, "no food");

    expectSubmit();
    _logger.LogFailure("worse problem", "no water", "serious", "#555", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("Failure"));
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Signature"].stringValue, "worse problem");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Detail"].stringValue, "no water");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Category"].stringValue, "serious");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Id"].stringValue, "#555");

    expectSubmit();
    EventProperties props("problem_report");
    props.SetProperty("life_support_eta", "7:30");
    _logger.LogFailure("catastrophic problem", "oxygen tank exploded", "deadly", "#666", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("Failure"));
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Signature"].stringValue, "catastrophic problem");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Detail"].stringValue, "oxygen tank exploded");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Category"].stringValue, "deadly");
    EXPECT_THAT(_submittedRecord.data[0].properties["Failure.Id"].stringValue, "#666");
    EXPECT_THAT(_submittedRecord.data[0].properties["life_support_eta"].stringValue, "7:30");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.ActionType"].stringValue, "Click");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.PageViewId"].stringValue, "/index.html");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.RawActionType"].stringValue, "Unspecified");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.InputDeviceType"].stringValue, "Unspecified");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemLayout.Rank"].longValue, 0);

    expectSubmit();
    PageActionData pad("/eshop/cart.aspx", ActionType_Zoom);
    pad.inputDeviceType = InputDeviceType_Touch;
    pad.rawActionType   = RawActionType_TouchZoom;
    _logger.LogPageAction(pad, _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.ActionType"].stringValue, "Zoom");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.PageViewId"].stringValue, "/eshop/cart.aspx");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.RawActionType"].stringValue, "TouchZoom");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.InputDeviceType"].stringValue, "Touch");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemLayout.Rank"].longValue, 0);

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageAction"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.ActionType"].stringValue, "Other");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.PageViewId"].stringValue, "SendMessage.action");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.RawActionType"].stringValue, "KeyboardPress");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.InputDeviceType"].stringValue, "Keyboard");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.DestinationUri"].stringValue, "SendAttachment.action");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemId"].stringValue, "attach");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemDataSource.Name"].stringValue, "extras");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemDataSource.Category"].stringValue, "form");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemDataSource.Collection"].stringValue, "buttons");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemLayout.Container"].stringValue, "message-entry");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageAction.TargetItemLayout.Rank"].longValue,123);
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Id"].stringValue, "id-1");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Name"].stringValue, "Index");

    expectSubmit();
    _logger.LogPageView("id-2", "Cart", "eshop", "/eshop/cart.aspx", "/eshop/index.aspx", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Id"].stringValue, "id-2");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Name"].stringValue, "Cart");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Category"].stringValue, "eshop");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Uri"].stringValue, "/eshop/cart.aspx");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.ReferrerUri"].stringValue, "/eshop/index.aspx");

    expectSubmit();
    EventProperties props("internal_browser_page_view");
    props.SetProperty("test", "value");
    _logger.LogPageView("id-3", "Order", "eshop", "/eshop/order.aspx", "/eshop/cart.aspx", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("PageView"));
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Id"].stringValue, "id-3");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Name"].stringValue, "Order");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Category"].stringValue, "eshop");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.Uri"].stringValue, "/eshop/order.aspx");
    EXPECT_THAT(_submittedRecord.data[0].properties["PageView.ReferrerUri"].stringValue, "/eshop/cart.aspx");
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Name"].stringValue, "measurement");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Value"].doubleValue, 1.200000);
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Units"].stringValue, "ms");

    expectSubmit();
    _logger.LogSampledMetric("speed", 67.89, "km/h", "Ferda", "vehicle", "id-1", _emptyProperties);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Name"].stringValue, "speed");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Value"].doubleValue, 67.890000);
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Units"].stringValue, "km/h");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.InstanceName"].stringValue, "Ferda");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.ObjectClass"].stringValue, "vehicle");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.ObjectId"].stringValue, "id-1");

    expectSubmit();
    EventProperties props("detected_vehicle");
    _logger.LogSampledMetric("speed", 89.01, "km/h", "Adref", "vehicle", "id-2", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("SampledMetric"));
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Name"].stringValue, "speed");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Value"].doubleValue, 89.010000);
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.Units"].stringValue, "km/h");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.InstanceName"].stringValue, "Adref");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.ObjectClass"].stringValue, "vehicle");
    EXPECT_THAT(_submittedRecord.data[0].properties["SampledMetric.ObjectId"].stringValue, "id-2");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("Trace"));
    EXPECT_THAT(_submittedRecord.data[0].properties["Trace.Level"].stringValue, "Information");
    EXPECT_THAT(_submittedRecord.data[0].properties["Trace.Message"].stringValue, "Everything is all right");

    expectSubmit();
    EventProperties props("worker_report");
    props.SetProperty("test", "value");
    _logger.LogTrace(TraceLevel_Warning, "Stuff is getting harder", props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("Trace"));
    EXPECT_THAT(_submittedRecord.data[0].properties["Trace.Level"].stringValue,"Warning");
    EXPECT_THAT(_submittedRecord.data[0].properties["Trace.Message"].stringValue, "Stuff is getting harder");
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue, "value");

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
    EXPECT_THAT(_submittedRecord.baseType, Eq("UserInfo_UserState"));
    EXPECT_THAT(_submittedRecord.data[0].properties["State.Name"].stringValue ,        "UserState");
    EXPECT_THAT(_submittedRecord.data[0].properties["State.Value"].stringValue ,       "Connected");
    EXPECT_THAT(_submittedRecord.data[0].properties["State.TimeToLive"].longValue, 12345);
    EXPECT_THAT(_submittedRecord.data[0].properties["State.IsTransition"].longValue, true);

    expectSubmit();
    EventProperties props("app_login_state");
    props.SetProperty("test", "value");
    _logger.LogUserState(UserState_SignedIn, 67890, props);
    ASSERT_THAT(_submitted, true);
    checkBaseAndContextAndRuntimeConfigProps();
    EXPECT_THAT(_submittedRecord.baseType, Eq("UserInfo_UserState"));
    EXPECT_THAT(_submittedRecord.data[0].properties["State.Name"].stringValue,         "UserState");
    EXPECT_THAT(_submittedRecord.data[0].properties["State.Value"].stringValue ,       "SignedIn");
    EXPECT_THAT(_submittedRecord.data[0].properties["State.TimeToLive"].longValue,  67890);
    EXPECT_THAT(_submittedRecord.data[0].properties["State.IsTransition"].longValue, true);
    EXPECT_THAT(_submittedRecord.data[0].properties["test"].stringValue,              "value");
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
    ::AriaProtocol::CsEvent record;
    record.baseType = "off";
	std::string name("test");
	std::uint64_t flags = 0;
    _logger.submit_(record, EventPriority_Off, flags);
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
    _logger.submit_(record, EventPriority_Unspecified,0);

    //EXPECT_THAT(event->record.id,          Eq("guid"));
    EXPECT_THAT(event->record.tenantToken, Eq("testtenantid-tenanttoken"));
    EXPECT_THAT(event->record.priority,    EventPriority_Unspecified);
    EXPECT_THAT(event->source->name,         Eq("guid"));
    EXPECT_THAT(event->source->baseType,  Eq("eventtype"));
    EXPECT_THAT(event->source->data[0].properties["propertykey"].stringValue,"propertyvalue");
}

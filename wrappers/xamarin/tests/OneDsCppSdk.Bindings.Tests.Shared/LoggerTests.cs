//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
extern alias NUnitLite;

using Microsoft.Applications.Events;
using NUnitLite::NUnit.Framework;

#if __ANDROID__
using Android.App;
using Java.Util;
#endif

#if __IOS__
using Foundation;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public class LoggerTests : BaseTests
    {
        private const string Token = "fake-token";
        private const string TestEventName = "eventname";

        private EventProperties TestEventProperties;

        private ILogger logger;

        [SetUp]
        public void Setup()
        {
            logger = LogManager.InitializeLogger(Token);
            TestEventProperties = new EventProperties(TestEventName);
        }

        [TearDown]
        public void Tear() { }

        [Test]
        public void GetSemanticContext()
        {
            ISemanticContext semanticContext;
#if __ANDROID__
            semanticContext = logger.SemanticContext;
#elif __IOS__
            semanticContext = logger.GetSemanticContext();
#endif
            Assert.NotNull(semanticContext);
        }

        [Test]
        public void LogEventWithName()
        {
            ExecuteWithoutAssertion(() => logger.LogEvent(TestEventName));
        }

        [Test]
        public void LogEventWithProperties()
        {
            ExecuteWithoutAssertion(() => logger.LogEvent(TestEventProperties));
        }

        [Test]
        public void LogFailure()
        {
            ExecuteWithoutAssertion(() => logger.LogFailure("signature", "detail", TestEventProperties));
        }

        [Test]
        public void LogFailureWithCategory()
        {
            ExecuteWithoutAssertion(() => logger.LogFailure("signature", "detail", "category", "identifier", TestEventProperties));
        }

        [Test]
        public void LogPageView()
        {
            ExecuteWithoutAssertion(() => logger.LogPageView("identifier", "pageName", TestEventProperties));
        }

        [Test]
        public void LogPageViewWithCategory()
        {
            ExecuteWithoutAssertion(() => logger.LogPageView("identifier", "pageName", "category", "uri", "referrerUri", TestEventProperties));
        }

        [Test]
        public void LogSession()
        {
            ExecuteWithoutAssertion(() => logger.LogSession(SessionState.Started, TestEventProperties));
        }

        [Test]
        public void LogTrace()
        {
            ExecuteWithoutAssertion(() => logger.LogTrace(TraceLevel.Error, "message", TestEventProperties));
        }

        [Test]
        public void SetContextWithStringValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", "value", PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithStringValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", "value"));
        }

        [Test]
        public void SetContextWithMaxDoubleValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", double.MaxValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMaxDoubleValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", double.MaxValue));
        }

        [Test]
        public void SetContextWithMinDoubleValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", double.MinValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMinDoubleValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", double.MinValue));
        }

        [Test]
        public void SetContextWithMaxInt64ValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", long.MaxValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMaxInt64Value()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", long.MaxValue));
        }

        [Test]
        public void SetContextWithMinInt64ValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", long.MinValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMinInt64Value()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", long.MinValue));
        }

        [Test]
        public void SetContextWithMaxInt32ValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", int.MaxValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMaxInt32Value()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", int.MaxValue));
        }

        [Test]
        public void SetContextWithMinInt32ValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", int.MinValue, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithMinInt32Value()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", int.MinValue));
        }

        [Test]
        public void SetContextWithBoolValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", true, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithBoolValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", true));
        }

        [Test]
        public void SetContextWithGuidValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", DataTypes.TestGuid, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithGuidValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", DataTypes.TestGuid));
        }

        [Test]
        public void SetContextWithDateTimeValueWithPiiKind()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", DataTypes.TestDate, PiiKind.DistinguishedName));
        }

        [Test]
        public void SetContextWithDateTimeValue()
        {
            ExecuteWithoutAssertion(() => logger.SetContext("name", DataTypes.TestDate));
        }
    }
}

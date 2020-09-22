using Foundation;
using Microsoft.Applications.Events;
using NUnit.Framework;
using System;

#if __ANDROID__
using Android.App;
#endif

namespace OneDsCppSdk.Bindings.Tests.Shared
{
    [TestFixture]
    public class LoggerTests
    {
        private const string Token = "fake-token";
        private const string TestEventName = "eventname";

        private readonly EventProperties TestEventProperties = new EventProperties(TestEventName);

        private ILogger logger;

        [SetUp]
        public void Setup()
        {
#if __ANDROID__
            logger = LogManager.Initialize(Application.Context, Token);
#elif __IOS__
            logger = LogManager.Initialize(Token);
#endif
        }

        [TearDown]
        public void Tear() { }

        [Test]
        public void InstantiateLogger()
        {
            Assert.NotNull(logger);
        }

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
            logger.LogEvent(TestEventName);
        }

        [Test]
        public void LogEventWithProperties()
        {
            logger.LogEvent(TestEventProperties);
        }

        [Test]
        public void LogFailure()
        {
            logger.LogFailure("signature", "detail", TestEventProperties);
        }

        [Test]
        public void LogFailureWithCategory()
        {
            logger.LogFailure("signature", "detail", "category", "identifier", TestEventProperties);
        }

        [Test]
        public void LogPageView()
        {
            logger.LogPageView("identifier", "pageName", TestEventProperties);
        }

        [Test]
        public void LogPageViewWithCategory()
        {
            logger.LogPageView("identifier", "pageName", "category", "uri", "referrerUri", TestEventProperties);
        }

        [Test]
        public void LogSession()
        {
            logger.LogSession(SessionState.Started, TestEventProperties);
        }

        [Test]
        public void LogTrace()
        {
            logger.LogTrace(TraceLevel.Error, "message", TestEventProperties);
        }

        [Test]
        public void SetContextWithStringValueWithPiiKind()
        {
            logger.SetContext("name", "value", PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithStringValue()
        {
            logger.SetContext("name", "value");
        }

        [Test]
        public void SetContextWithMaxDoubleValueWithPiiKind()
        {
            logger.SetContext("name", double.MaxValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMaxDoubleValue()
        {
            logger.SetContext("name", double.MaxValue);
        }

        [Test]
        public void SetContextWithMinDoubleValueWithPiiKind()
        {
            logger.SetContext("name", double.MinValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMinDoubleValue()
        {
            logger.SetContext("name", double.MinValue);
        }

        [Test]
        public void SetContextWithMaxInt64ValueWithPiiKind()
        {
            logger.SetContext("name", long.MaxValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMaxInt64Value()
        {
            logger.SetContext("name", long.MaxValue);
        }

        [Test]
        public void SetContextWithMinInt64ValueWithPiiKind()
        {
            logger.SetContext("name", long.MinValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMinInt64Value()
        {
            logger.SetContext("name", long.MinValue);
        }

        [Test]
        public void SetContextWithMaxInt32ValueWithPiiKind()
        {
            logger.SetContext("name", int.MaxValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMaxInt32Value()
        {
            logger.SetContext("name", int.MaxValue);
        }

        [Test]
        public void SetContextWithMinInt32ValueWithPiiKind()
        {
            logger.SetContext("name", int.MinValue, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithMinInt32Value()
        {
            logger.SetContext("name", int.MinValue);
        }

        [Test]
        public void SetContextWithBoolValueWithPiiKind()
        {
            logger.SetContext("name", true, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithBoolValue()
        {
            logger.SetContext("name", true);
        }

        [Test]
        public void SetContextWithGuidValueWithPiiKind()
        {
            logger.SetContext("name", Guid.Empty, PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithGuidValue()
        {
            logger.SetContext("name", Guid.Empty);
        }

         [Test]
        public void SetContextWithDateTimeValueWithPiiKind()
        {
            logger.SetContext("name", ToNsDate(DateTime.MinValue), PiiKind.DistinguishedName);
        }

        [Test]
        public void SetContextWithDateTimeValue()
        {
            logger.SetContext("name", ToNsDate(DateTime.MinValue));
        }

        private NSDate ToNsDate(DateTime dateTime)
        {
            if (dateTime.Kind == DateTimeKind.Unspecified)
            {
                dateTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Local);
            }
            return (NSDate)dateTime;
        }
    }
}
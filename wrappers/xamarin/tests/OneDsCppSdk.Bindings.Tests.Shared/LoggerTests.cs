using Microsoft.Applications.Events;
using NUnit.Framework;

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
    }
}
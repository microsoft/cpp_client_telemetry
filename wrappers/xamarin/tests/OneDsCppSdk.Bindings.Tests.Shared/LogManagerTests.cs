using Microsoft.Applications.Events;
using NUnit.Framework;

#if __ANDROID__
using Android.App;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public partial class LogManagerTests
    {
        private const string Token = "fake-token";

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

        [Test]
        public void InstantiateLogger()
        {
            Assert.NotNull(logger);
        }
    }
}
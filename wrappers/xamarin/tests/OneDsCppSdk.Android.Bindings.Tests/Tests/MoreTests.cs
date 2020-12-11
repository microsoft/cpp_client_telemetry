using Microsoft.Applications.Events;
using NUnit.Framework;

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public class MoreTests : BaseTests
    {
        [TearDown]
        public void Tear() { }

        [Test]
        public void TestStuff()
        {
            var config = LogManager.LogConfigurationFactory();
            config.Set(LogConfigurationKey.CfgStrCollectorUrl, "https://tb.events.data.microsoft.com/OneCollector/1.0/");
            var logger = LogManager.Initialize("a994760b15ea4324a40bd0dc7084bb4e-7d405dae-1957-4b45-b571-383a6be3848f-7217", config);
            logger.LogEvent("client.info");
        }
    }
}

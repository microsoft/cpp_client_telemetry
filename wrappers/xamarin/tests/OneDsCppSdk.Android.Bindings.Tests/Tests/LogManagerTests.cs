extern alias NUnitLite;

using Microsoft.Applications.Events;
using NUnitLite::NUnit.Framework;
using OneDsCppSdk.NetStandard.Tests;
using System.Threading.Tasks;

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public class LogManagerTests : BaseTests
    {
        [Test]
        public void InstantiateLoggerWithToken()
        {
            LogManagerTestMethods.InstantiateLoggerWithToken();
        }

        [Test]
        public void InstantiateLoggerWithTokenAndConfig()
        {
            LogManagerTestMethods.InstantiateLoggerWithTokenAndConfig();
        }

        [Test]
        public void Flush()
        {
            LogManagerTestMethods.PauseTransmission();
        }

        [Test]
        public async void FlushAndTeardown()
        {
            LogManagerTestMethods.FlushAndTeardown();
        }

        [Test]
        public void PauseTransmission()
        {
            LogManagerTestMethods.PauseTransmission();
        }

        [Test]
        public void ResumeTransmission()
        {
            LogManagerTestMethods.ResumeTransmission();
        }
    }
}

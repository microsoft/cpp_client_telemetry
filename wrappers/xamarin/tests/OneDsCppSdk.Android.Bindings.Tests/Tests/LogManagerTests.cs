using Microsoft.Applications.Events;
using NUnit.Framework;
using System.Threading.Tasks;

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public partial class LogManagerTests
    {
        [Test]
        public async void Flush()
        {
            Status status = null;
            await Task.Run(() =>
            {
                status = LogManager.Flush();
            });

            Assert.AreEqual(Status.Success, status);
        }

        [Test]
        public async void FlushAndTeardown()
        {
            Status status = null;
            await Task.Run(() =>
            {
                status = LogManager.FlushAndTeardown();
            });

            Assert.AreEqual(Status.Success, status);
        }
    }
}
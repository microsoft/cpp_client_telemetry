using Microsoft.Applications.Events;
using NUnit.Framework;

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public partial class LogManagerTests
    {
        [Test]
        public void Flush()
        {
            LogManager.Flush();
        }
    }
}
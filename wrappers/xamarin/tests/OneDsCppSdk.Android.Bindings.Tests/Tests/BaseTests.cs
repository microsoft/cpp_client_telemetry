using Android.App;
using Microsoft.Applications.Events;
using NUnit.Framework;

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public abstract class BaseTests
    {
        [SetUp]
        public void Setup()
        {
            Library.InitializeLibrary(Application.Context);
        }
    }
}

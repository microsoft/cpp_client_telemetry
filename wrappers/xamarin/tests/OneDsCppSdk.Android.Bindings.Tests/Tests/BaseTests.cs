extern alias NUnitLite;

using Android.App;
using Microsoft.Applications.Events;
using NUnitLite::NUnit.Framework;
using System;

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

        protected void ExecuteWithoutAssertion(Action action)
        {
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Assert.Fail(ex.Message, ex.StackTrace);
            }

            Assert.Pass("Executed successfully");
        }
    }
}

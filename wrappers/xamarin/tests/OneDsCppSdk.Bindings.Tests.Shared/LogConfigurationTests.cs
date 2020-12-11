////
//// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
//// SPDX-License-Identifier: Apache-2.0
////
//using Microsoft.Applications.Events;
//using NUnit.Framework;
//using System.Threading.Tasks;

//#if __ANDROID__
//using Android.App;
//#endif

//namespace OneDsCppSdk.Bindings.Tests
//{
//    [TestFixture]
//    public partial class LogConfigurationTests : BaseTests
//    {
//        [Test]
//        public void SetAndGetEventCollectorUri()
//        {
//            var expectedUri = "https://someuri";

//            LogConfiguration.EventCollectorUri = expectedUri;

//            var actualUri = LogConfiguration.EventCollectorUri;
//            Assert.AreEqual(expectedUri, actualUri);
//        }

//        [Test]
//        public void SetAndGetCacheFilePath()
//        {
//            var expectedPath = "somepath";

//            LogConfiguration.CacheFilePath = expectedPath;

//            var actualPath = LogConfiguration.CacheFilePath;
//            Assert.AreEqual(expectedPath, actualPath);
//        }
//    }
//}

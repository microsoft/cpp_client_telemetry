//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Microsoft.Applications.Events;
using NUnit.Framework;
using System.Threading.Tasks;

#if __ANDROID__
using Android.App;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public partial class LogManagerTests : BaseTests
    {
        private const string Token = "fake-token";

        private ILogger logger;

        [SetUp]
        public void Setup()
        {
            logger = LogManager.InitializeLogger(Token);
        }

        [Test]
        public void InstantiateLogger()
        {
            Assert.NotNull(logger);
        }

        [Test]
        public async void Flush()
        {
            Status? status = null;
            await Task.Run(() =>
            {
                status = LogManager.Flush();
            });

            Assert.AreEqual(Status.Success, status);
        }

        [Test]
        public async void FlushAndTeardown()
        {
            Status? status = null;
            await Task.Run(() =>
            {
                status = LogManager.FlushAndTeardown();
            });

            Assert.AreEqual(Status.Success, status);
        }

        [Test]
        public void PauseTransmission()
        {
            LogManager.PauseTransmission();
        }

        [Test]
        public void ResumeTransmission()
        {
            LogManager.ResumeTransmission();
        }
    }
}

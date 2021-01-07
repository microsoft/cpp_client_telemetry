//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
extern alias NUnitLite;

using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.Applications.Events;
using NUnitLite::NUnit.Framework;

#if __ANDROID__
using Android.App;
#elif __IOS__
using Foundation;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    [TestFixture]
    public partial class LogManagerTests : BaseTests
    {
        private const string Token = "fake-token";

        [Test]
        public void InstantiateLoggerWithToken()
        {
            var logger = LogManager.InitializeLogger(Token);
            Assert.NotNull(logger);
        }

        [Test]
        public async void Flush()
        {
            var logger = LogManager.InitializeLogger(Token);

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
            var logger = LogManager.InitializeLogger(Token);

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
            ExecuteWithoutAssertion(() => LogManager.PauseTransmission());
        }

        [Test]
        public void ResumeTransmission()
        {
            ExecuteWithoutAssertion(() => LogManager.ResumeTransmission());
        }
    }
}

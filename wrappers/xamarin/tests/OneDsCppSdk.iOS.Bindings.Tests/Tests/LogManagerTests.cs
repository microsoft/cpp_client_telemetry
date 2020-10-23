//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Microsoft.Applications.Events;
using NUnit.Framework;

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
            logger = LogManager.Initialize(Token);
        }

        [Test]
        public void InstantiateLogger()
        {
            Assert.NotNull(logger);
        }

        [Test]
        public void Flush()
        {
            LogManager.Flush();
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
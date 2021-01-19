//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
extern alias NUnitLite;

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

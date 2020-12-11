//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Microsoft.Applications.Events;
using NUnit.Framework;
using System;
using System.Threading.Tasks;

namespace OneDsCppSdk.NetStandard.Tests
{
    public static class LogManagerTestMethods
    {
        private const string Token = "fake-token";

        public static void InstantiateLoggerWithToken()
        {
            var logger = LogManager.InitializeLogger(Token);
            Assert.NotNull(logger);
        }

        public static void InstantiateLoggerWithTokenAndConfig()
        {
            string uri = "myuri";
            string path = "mypath";

            var config = LogConfiguration.GetInstance();
            //config.ge
            //config.Set(LogConfigurationKey.CfgStrCollectorUrl, uri);
            //config.Set(LogConfigurationKey.CfgStrCacheFilePath, path);
            var logger = LogManager.InitializeLogger(Token, config);

            Assert.NotNull(logger);
        }

        public static void Flush()
        {
            var logger = LogManager.InitializeLogger(Token);

            Status? status = null;
            //await Task.Run(() =>
            //{
                status = LogManager.Flush();
            //});

            Assert.AreEqual(Status.Success, status);
        }

        public static async void FlushAndTeardown()
        {
            var logger = LogManager.InitializeLogger(Token);

            Status? status = null;
            await Task.Run(() =>
            {
                status = LogManager.FlushAndTeardown();
            });

            Assert.AreEqual(Status.Success, status);
        }

        public static void PauseTransmission()
        {
            ExecuteWithoutAssertion(() => LogManager.PauseTransmission());
        }

        public static void ResumeTransmission()
        {
            ExecuteWithoutAssertion(() => LogManager.ResumeTransmission());
        }

        private static void ExecuteWithoutAssertion(Action action)
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

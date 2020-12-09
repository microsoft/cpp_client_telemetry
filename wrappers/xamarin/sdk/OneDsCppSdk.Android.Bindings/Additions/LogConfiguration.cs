//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using Android.Content;
using Java.Lang;

namespace Microsoft.Applications.Events
{
    public class LogConfiguration
    {
        public static string EventCollectorUri
        {
            get
            {
                return LogManager.LogConfigurationFactory().GetString(LogConfigurationKey.CfgStrCollectorUrl);
            }
            set
            {
                LogManager.LogConfigurationFactory().Set(LogConfigurationKey.CfgStrCollectorUrl, value);
            }
        }

        public static string CacheFilePath
        {
            get
            {
                return LogManager.LogConfigurationFactory().GetString(LogConfigurationKey.CfgStrCacheFilePath);
            }
            set
            {
                LogManager.LogConfigurationFactory().Set(LogConfigurationKey.CfgStrCacheFilePath, value);
            }
        }
    }
}

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
namespace Microsoft.Applications.Events
{
    public partial class LogConfiguration //: ILogConfiguration
    {
        public static LogConfiguration GetInstance()
        {
            return new LogConfiguration();
        }

        public void Set(string key, string value)
        {
            //switch (key)
            //{
            //    case LogConfigurationKey.CfgStrCollectorUrl:
            //        LogConfiguration.EventCollectorUri = value;
            //        break;
            //    case LogConfigurationKey.CfgStrCacheFilePath:
            //        LogConfiguration.CacheFilePath = value;
            //        break;
            //    default:
            //        return false;
            //}
        }
    }
}

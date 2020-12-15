//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Foundation;
using System.Collections.Generic;

namespace Microsoft.Applications.Events
{
    public partial class LogManager
    {
        public static Logger InitializeLogger(string tenantToken, IDictionary<string, string> config)
        {
            foreach (var configItem in config)
            {
                switch (configItem.Key)
                {
                    case LogConfigurationKey.CfgStrCacheFilePath:
                        LogConfiguration.CacheFilePath = configItem.Value;
                        break;
                    case LogConfigurationKey.CfgStrCollectorUrl:
                        LogConfiguration.EventCollectorUri = configItem.Value;
                        break;
                }
            }

            return InitializeLogger(tenantToken);
        }
    }
}

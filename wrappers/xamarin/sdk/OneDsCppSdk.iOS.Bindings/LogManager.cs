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
        //public static Logger InitializeLogger(string tenantToken, LogConfiguration config)
        public static Logger InitializeLogger(string tenantToken, Dictionary<string, string> config)
        {
            var configDictionary = new NSDictionary(LogConfigurationKey.CfgStrCollectorUrl, config[LogConfigurationKey.CfgStrCollectorUrl], LogConfigurationKey.CfgStrCacheFilePath, config[LogConfigurationKey.CfgStrCollectorUrl]);
            return InitializeLogger(tenantToken, configDictionary);
        }
    }
}

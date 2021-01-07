//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System.Collections.Generic;

namespace Microsoft.Applications.Events
{
    public partial class LogManager
    {
        public static Logger InitializeLogger(string tenantToken)
        {
            return (Logger)Initialize(tenantToken);
        }

        public static Logger InitializeLogger(string tenantToken, IDictionary<string, string> logConfiguration)
        {
            var configuration = LogManager.LogConfigurationFactory();
            foreach (var config in logConfiguration)
            {
                configuration.Set(config.Key, config.Value);
            }

            return (Logger)Initialize(tenantToken, configuration);
        }
    }
}

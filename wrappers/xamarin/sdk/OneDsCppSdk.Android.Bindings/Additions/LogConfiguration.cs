//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using static Microsoft.Applications.Events.LogManager;

namespace Microsoft.Applications.Events
{
    public partial class LogConfiguration
    {
        public static ILogConfiguration GetInstance()
        {
            var lconf = LogManager.LogConfigurationFactory();
            return lconf;
        }
    }
}

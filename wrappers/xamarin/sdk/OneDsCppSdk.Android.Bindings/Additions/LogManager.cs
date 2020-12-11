//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
namespace Microsoft.Applications.Events
{
    public partial class LogManager
    {
        public static Logger InitializeLogger(string tenantToken)
        {
            return (Logger)Initialize(tenantToken);
        }

        public static Logger InitializeLogger(string tenantToken, ILogConfiguration logConfiguration)
        {
            return (Logger)Initialize(tenantToken, logConfiguration);
        }
    }
}

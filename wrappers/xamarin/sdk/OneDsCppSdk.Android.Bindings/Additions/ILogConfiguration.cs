//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;

namespace Microsoft.Applications.Events
{
    public partial class ILogConfiguration
    {
        public void Set(string key, string value)
        {
            Set(key, new Java.Lang.String(value));
        }
    }
}

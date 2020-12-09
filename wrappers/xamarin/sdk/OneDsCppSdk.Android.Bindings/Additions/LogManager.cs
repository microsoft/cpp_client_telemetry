//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Applications.Events
{
    public partial class LogManager
    {
        public static Logger InitializeLogger(string tenantToken)
        {
            return (Logger)Initialize(tenantToken);
        }
    }
}

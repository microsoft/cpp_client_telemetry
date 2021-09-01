//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
using Android.Content;
using Java.Lang;
using System;

namespace Microsoft.Applications.Events
{
    public static class Library
    {
        public static void InitializeLibrary(Context context)
        {
            if (context == null)
            {
                throw new ArgumentNullException(nameof(context), "context cannot be null");
            }

            JavaSystem.LoadLibrary("maesdk");
            HttpClient httpClient = new HttpClient(context);
            OfflineRoom.ConnectContext(context);
        }
    }
}

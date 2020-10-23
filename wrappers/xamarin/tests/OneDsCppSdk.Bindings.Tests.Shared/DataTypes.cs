//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#if __ANDROID__
using Java.Util;
using Microsoft.Applications.Events;
using System.Collections.Generic;
#elif __IOS__
using Foundation;
#endif

namespace OneDsCppSdk.Bindings.Tests
{
    public static class DataTypes
    {
#if __ANDROID__
        public static IDictionary<string, EventProperty> TestProperties => new Dictionary<string, EventProperty>();
        public static UUID TestGuid => UUID.RandomUUID();
        public static Date TestDate => new Date();
#elif __IOS__
        public static NSDictionary<NSString, NSObject> TestProperties => new NSDictionary<NSString, NSObject>();
        public static NSUuid TestGuid => new NSUuid();
        public static NSDate TestDate => new NSDate();
#endif
    }
}

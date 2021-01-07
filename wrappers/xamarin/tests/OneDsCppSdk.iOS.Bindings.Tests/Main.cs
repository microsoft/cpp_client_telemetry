//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using UIKit;

namespace OneDsCppSdk.iOS.Bindings.Tests
{
    public class Application
    {
        // This is the main entry point of the application.
        static void Main(string[] args)
        {
            // if you want to use a different Application Delegate class from "UnitTestAppDelegate"
            // you can specify it here.
            UIApplication.Main(args, null, "UnitTestAppDelegate");
        }
    }
}

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using System.Collections.Generic;

namespace Microsoft.Applications.Events
{
    public class LogManager
    {
        public static Logger InitializeLogger(string tenantToken) { throw new NotImplementedException(); }

        public static Logger InitializeLogger(string tenantToken, IDictionary<string, string> logConfiguration) { throw new NotImplementedException(); }

        public static void UploadNow() { }

        public static Status Flush() { throw new NotImplementedException(); }

        public static Status FlushAndTeardown() { throw new NotImplementedException(); }

        public static void SetTransmissionProfile(TransmissionProfile profile) { }

        public static void PauseTransmission() { }

        public static void ResumeTransmission() { }

        public static void ResetTransmitProfiles() { }
    }
}

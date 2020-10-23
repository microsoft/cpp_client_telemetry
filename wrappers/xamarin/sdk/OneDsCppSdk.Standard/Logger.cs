//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;

namespace Microsoft.Applications.Events
{
    public class Logger : ILogger
    {
        public ISemanticContext SemanticContext => default;

        public void LogEvent(string name) { }

        public void LogEvent(EventProperties properties) { }

        public void LogFailure(string signature, string detail, EventProperties properties) { }

        public void LogFailure(string signature, string detail, string category, string id, EventProperties properties) { }

        public void LogPageView(string identifier, string pageName, EventProperties properties) { }

        public void LogPageView(string identifier, string pageName, string category, string uri, string referrerUri, EventProperties properties) { }

        public void LogTrace(TraceLevel level, string message, EventProperties properties) { }

        public void LogSession(SessionState state, EventProperties properties) { }

        public void SetContext(string name, string value) { }

        public void SetContext(string name, string value, PiiKind piiKind) { }

        public void SetContext(string name, bool value) { }

        public void SetContext(string name, bool value, PiiKind piiKind) { }

        public void SetContext(string name, DateTime value) { }

        public void SetContext(string name, DateTime value, PiiKind piiKind) { }

        public void SetContext(string name, double value) { }

        public void SetContext(string name, double value, PiiKind piiKind) { }

        public void SetContext(string name, long value) { }

        public void SetContext(string name, long value, PiiKind piiKind) { }

        public void SetContext(string name, int value) { }

        public void SetContext(string name, int value, PiiKind piiKind) { }

        public void SetContext(string name, Guid value) { }

        public void SetContext(string name, Guid value, PiiKind piiKind) { }
    }
}

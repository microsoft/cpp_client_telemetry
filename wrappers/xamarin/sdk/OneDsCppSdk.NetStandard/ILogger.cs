//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;

namespace Microsoft.Applications.Events
{
    public interface ILogger
    {
        ISemanticContext SemanticContext { get; }

        void LogEvent(string name);

        void LogEvent(EventProperties properties);

        void LogFailure(string signature, string detail, EventProperties properties);

        void LogFailure(string signature, string detail, string category, string id, EventProperties properties);

        void LogPageView(string identifier, string pageName, EventProperties properties);

        void LogPageView(string identifier, string pageName, string category, string uri, string referrerUri, EventProperties properties);

        void LogTrace(TraceLevel level, string message, EventProperties properties);

        void LogSession(SessionState state, EventProperties properties);

        void SetContext(string name, string value);

        void SetContext(string name, string value, PiiKind piiKind);

        void SetContext(string name, bool value);

        void SetContext(string name, bool value, PiiKind piiKind);

        void SetContext(string name, DateTime value);

        void SetContext(string name, DateTime value, PiiKind piiKind);

        void SetContext(string name, double value);

        void SetContext(string name, double value, PiiKind piiKind);

        void SetContext(string name, long value);

        void SetContext(string name, long value, PiiKind piiKind);

        void SetContext(string name, int value);

        void SetContext(string name, int value, PiiKind piiKind);

        void SetContext(string name, Guid value);

        void SetContext(string name, Guid value, PiiKind piiKind);
    }
}

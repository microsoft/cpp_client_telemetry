//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System.Collections.Generic;

namespace Microsoft.Applications.Events
{
    public partial class EventProperties
    {
        public EventProperties (EventProperties copy) {}

        public EventProperties (string name) {}

        public void SetProperty(string name, string value) {}

        public EventProperties (string name, IDictionary<string, EventProperty> properties) {}

        public virtual string Name { get; }
    }
}

//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

let props = EventProperties(name:"TestEvent")
props.setProperty("PropName", withValue: ["Type":"SwiftWrappers"])
props.setProperty("PropWithPII", withInt64Value: Int64(30), withPiiKind: ODWPiiKind.distinguishedName)
print (props.properties)
print (props.piiTags)

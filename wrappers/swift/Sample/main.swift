//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import OneDSSwift // Swift Package containing the wrappers over ObjC

let props = EventProperties(name:"TestEvent")
props.setProperty("PropName", withValue: ["Type":"SwiftWrappers"])
props.setProperty("PropWithPII", withInt64Value: Int64(30), withPiiKind: PIIKind.distinguishedName)
print(props.properties())

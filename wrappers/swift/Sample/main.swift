//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule
import OneDSSwift


let props = EventProperties(name:"TestEvent")
props.setProperty("PropName", withValue: ["Type":"SwiftWrappers"])
props.setProperty("PropWithPII", withInt64Value: Int64(30), withPiiKind: ODWPiiKind.distinguishedName)
print(props.properties())

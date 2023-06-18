//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import OneDSSwift // Swift Package containing the wrappers over ObjC

let props = EventProperties(name:"TestEvent")
props.setProperty("PropName", withValue: ["Type":"SwiftWrappers"])
props.setProperty("PropWithPII", withInt64Value: Int64(30), withPiiKind: PIIKind.distinguishedName)
print(props.properties())

let token: String = "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"
let myLogger: Logger = LogManager.loggerWithTenant(tenantToken: token)!

let pgInitConfig : PrivacyGuardInitConfig = PrivacyGuardInitConfig()
pgInitConfig.getODWPrivacyGuardInitConfig().useEventFieldPrefix = false

print(pgInitConfig.getODWPrivacyGuardInitConfig().useEventFieldPrefix)
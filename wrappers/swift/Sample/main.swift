//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import Foundation

import OneDSSwift // Swift Package containing the wrappers over ObjC

let token: String = "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"
let myLogger: Logger? = LogManager.loggerWithTenant(tenantToken: token)
let pgInitConfig : PrivacyGuardInitConfig = PrivacyGuardInitConfig()

pgInitConfig.useEventFieldPrefix = true
pgInitConfig.dataContext.domainName = "TEST.MICROSOFT.COM"
pgInitConfig.dataContext.machineName = "Motherboard"
pgInitConfig.dataContext.userNames = [Any]()
pgInitConfig.dataContext.userNames.append("Awesome Username")
pgInitConfig.dataContext.userAliases = [Any]()
pgInitConfig.dataContext.userAliases.append("awesomeuser")
pgInitConfig.dataContext.IPAddresses = [Any]()
pgInitConfig.dataContext.IPAddresses.append("10.0.1.1")
pgInitConfig.dataContext.IPAddresses.append("192.168.1.1")
pgInitConfig.dataContext.IPAddresses.append("1234:4578:9abc:def0:bea4:ca4:ca1:d0g")
pgInitConfig.dataContext.languageIdentifiers = [Any]()
pgInitConfig.dataContext.languageIdentifiers.append("en-US")
pgInitConfig.dataContext.languageIdentifiers.append("English (United States)")
pgInitConfig.dataContext.machineIDs = [Any]()
pgInitConfig.dataContext.machineIDs.append("0450fe66-aeed-4059-99ca-4dd8702cbd1f")
pgInitConfig.dataContext.outOfScopeIdentifiers = [Any]()
pgInitConfig.dataContext.outOfScopeIdentifiers.append("43efb3b1-c7a3-4f29-beea-63ccb28160ac")
pgInitConfig.dataContext.outOfScopeIdentifiers.append("7d06a83a-200d-4ccb-bfc6-d0995c840bde")
pgInitConfig.dataContext.outOfScopeIdentifiers.append("e1b2ece8-2451-4ea9-997a-6f37b50be8de")

if (myLogger != nil) {
    myLogger!.apply(config: pgInitConfig)
    myLogger!.logEvent(name: "Simple_Swift_Event")
}

LogManager.uploadNow()

let event: EventProperties = EventProperties(name: "WEvtProps_Swift_Event",
                                        properties: [
                                            "result": "Success",
                                            "seq": 2,
                                            "random": 3,
                                            "secret": 5.75
                                        ])

let anotherLogger: Logger? = LogManager.loggerForSource(source: "anotherSource")

if (anotherLogger != nil) {
    anotherLogger!.logEvent(properties: event)
}

LogManager.uploadNow()

let event2: EventProperties = EventProperties(name: "SetProps_Swift_Event")
event2.setProperty("result", withValue: "Failure")
event2.setProperty("intVal", withInt64Value: Int64(8165))
event2.setProperty("doubleVal", withDoubleValue: Double(1.24))
event2.setProperty("wasSuccessful", withBoolValue: true)
event2.setProperty("myDate", withDateValue: Date())
event2.setProperty("transactionID", withUUIDValue: UUID(uuidString: "DEADBEEF-1234-2345-3456-123456789ABC")!)

anotherLogger!.logEvent(properties: event2)

anotherLogger!.semanticContext.setAppID("MyAppID")
anotherLogger!.semanticContext.setUserID("m:10101010101010101010")
anotherLogger!.semanticContext.setUserAdvertisingID("p:00000000-0000-0000-0000-000000000000")

let event3: EventProperties = EventProperties(name: "SematicContext_Swift_Event")
anotherLogger!.logEvent(properties: event3)
anotherLogger!.logEvent(name :"SemanticContext_Swift_EmtpyEvent")
_ = LogManager.flushAndTeardown()
PrivacyGuard.resetPrivacyGuardInstance()

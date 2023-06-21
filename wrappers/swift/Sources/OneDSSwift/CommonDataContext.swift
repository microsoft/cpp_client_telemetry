//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

/// Wrapper over ODWCommonDataContext class.
public final class CommonDataContext {
    ///
    public let odwCommonDataContext: ODWCommonDataContext

    /// Contructor initialized with ObjC wrapped object.
    public init(odwCommonDataContext: ODWCommonDataContext) {
        self.odwCommonDataContext = odwCommonDataContext
    }

    /// Domain Name for the current machine.
    public var domainName:String {
        get {
            odwCommonDataContext.domainName
        }
        set {
            odwCommonDataContext.domainName = newValue
        }
    }

    /// Friendly Machine Name
    public var machineName:String {
        get {
            odwCommonDataContext.machineName
        }
        set {
            odwCommonDataContext.machineName = newValue
        }
    }

    /// User Name such as the Login Name.
    public var userNames:[Any] {
        get {
            odwCommonDataContext.userNames as? [Any] ?? []
        }
        set {
            odwCommonDataContext.userNames = NSMutableArray(array: newValue)
        }
    }

    /// User Alias, if different that UserName.
    public var userAliases:[Any] {
        get {
            odwCommonDataContext.userAliases as? [Any] ?? []
        }
        set {
            odwCommonDataContext.userAliases = NSMutableArray(array: newValue)
        }
    }

    /// IP Addresses for local network ports such as IPv4, IPv6, etc.
    public var IPAddresses:[Any] {
        get {
            odwCommonDataContext.IPAddresses as? [Any] ?? []
        }
        set {
            odwCommonDataContext.IPAddresses = NSMutableArray(array: newValue)
        }
    }

    /// Collection of Language Identifiers.
    public var languageIdentifiers:[Any] {
        get {
            odwCommonDataContext.languageIdentifiers as? [Any] ?? []
        }
        set {
            odwCommonDataContext.languageIdentifiers = NSMutableArray(array: newValue)
        }
    }

    /// Collection of Machine Identifies such as Machine Name, Motherboard ID, MAC Address, etc.
    public var machineIDs:[Any] {
        get {
            odwCommonDataContext.machineIDs as? [Any] ?? []
        }
        set {
            odwCommonDataContext.machineIDs = NSMutableArray(array: newValue)
        }
    }

    /// Collection of Out-of-Scope identifiers such as Client ID, etc.
    public var outOfScopeIdentifiers:[Any] {
        get {
            odwCommonDataContext.outOfScopeIdentifiers as? [Any] ?? []
        }
        set {
            odwCommonDataContext.outOfScopeIdentifiers = NSMutableArray(array: newValue)
        }
    }
}

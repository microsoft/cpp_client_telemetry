//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

/// Wrapper over ODWPrivacyConcernMetadataProvider class.
public final class PrivacyConcernMetadataProvider {
    
    /// ObjC variable which is wrapped by Swift.
    var odwPrivacyConcernMetadataProvider: ODWPrivacyConcernMetadataProvider

    /// Constructor initialized with ObjC wrapped object.
    init(odwPrivacyConcernMetadataProvider: ODWPrivacyConcernMetadataProvider) {
        self.odwPrivacyConcernMetadataProvider = odwPrivacyConcernMetadataProvider
    }

    /// Default constructor.
    public init() {
        odwPrivacyConcernMetadataProvider = ODWPrivacyConcernMetadataProvider()
    }

    /// Get the database name for the provided record.
    public func getDatabaseName(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getDatabaseNameForRecord(record)
    }

    /// Get the server name for the provided record.
    public func getServerName(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getServerNameForRecord(record)
    }

    /// Get the event locator name for the provided record.
    public func getEventLocatorName(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getEventLocatorNameForRecord(record)
    }

    /// Get the event locator value for the provided record.
    public func getEventLocatorValue(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getEventLocatorValueForRecord(record)
    }

    /// Get the override for the privacy guard event time for the provided record.
    public func getPrivacyGuardEventTimeOverride(for record: Any) -> Int64 {
        return odwPrivacyConcernMetadataProvider.getPrivacyGuardEventTimeOverrideForRecord(record)
    }

    /// Check if the record should be ignored.
    public func getShouldIgnoreOverride(for record: Any) -> Bool {
        return odwPrivacyConcernMetadataProvider.getShouldIgnoreOverrideForRecord(record)
    }

    /// Get the associated tenant for the provided record.
    public func getAssociatedTenant(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getAssociatedTenantForRecord(record)
    }

    /// Get the environment for the provided record.
    public func getEnvironment(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getEnvironmentForRecord(record)
    }

    /// Get the metadata for the provided record.
    public func getMetadata(for record: Any) -> String {
        return odwPrivacyConcernMetadataProvider.getMetadataForRecord(record)
    }
}

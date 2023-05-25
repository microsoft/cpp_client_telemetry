//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/**
Represents Event's properties.
*/
public final class EventProperties {
    /// Obj-C class object which swift is wrapped around.
    private var odwEventProperties: ODWEventProperties

    /**
    Constructs EventProperties object with event name.

    - Parameters:
        - name: Name of the event as a `String`.
    */
    public init(name: String) {
        self.odwEventProperties = ODWEventProperties(name: name)
    }

    /**
    Constructs EventProperties object with event name and initial properties.

    - Parameters:
        - name: Name of the event.
        - properties: Initial properties dictionary.
    */
    public init(name: String, properties: [String: Any]) {
        self.odwEventProperties = ODWEventProperties(name: name, properties: properties)
    }

    /**
    Constructs EventProperties object with event name, initial properties and PII tags.

    - Parameters:
        - name: Name of the event.
        - properties: Initial properties dictionary.
        - piiTags: Initial PII tags.
    */
    public init(name: String, withProperties properties: [String: Any], withPiiTags piiTags: [String: NSNumber]) {
        self.odwEventProperties = ODWEventProperties(name: name, properties: properties, piiTags: piiTags)
    }

    /// Returns ODWEventProperties object, around which the Swift class is wrapped.
    public func getODWEventPropertiesObject() -> ODWEventProperties {
        return self.odwEventProperties
    }

    /// Returns all the properties as dictionary
    public func properties() -> [String: Any] {
        return self.odwEventProperties.properties
    }

    /// Returns all the piiTags as dictionary
    public func piiTags() -> [String: Any] {
        return self.odwEventProperties.piiTags
    }

    /**
    Sets the base type of the event, populated in Records.Type.

    - Parameters:
        - type: Type of the event.
    */
    public func setType(_ type: String) {
        self.odwEventProperties.setType(type)
    }

    /**
    Sets a string property for an event.

    - Parameters:
        - name: Name of the property.
        - value: Value of the property.
    */
    public func setProperty(_ name: String, withValue value: Any) {
        self.odwEventProperties.setProperty(name, withValue: value)
    }

    /**
    Sets a property for an event with PII tags.

    - Parameters:
        - name: Name of the property.
        - value: Value of the property.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withValue value: Any, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withValue: value, with: piiKind)
    }

    /**
    Sets a `Double` property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A Double that contains the property value.
    */
    public func setProperty(_ name: String, withDoubleValue value: Double) {
        self.odwEventProperties.setProperty(name, withDoubleValue: value)
    }

    /**
    Sets a `Double` property for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A Double that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withDoubleValue value: Double, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withDoubleValue: value, with: piiKind)
    }

    /**
    Sets an `Integer` property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withInt64Value value: Int64) {
        self.odwEventProperties.setProperty(name, withInt64Value: value)
    }

    /**
    Sets an `Integer` property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withInt64Value value: Int64, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withInt64Value: value, with: piiKind)
    }

    /**
    Sets an `Integer` property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withUInt8Value value: UInt8) {
        self.odwEventProperties.setProperty(name, withUInt8Value: value)
    }

    /**
    Sets an `Integer` property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUInt8Value value: UInt8, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withUInt8Value: value, with: piiKind)
    }

    /**
    Sets an `Integer` property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withUInt64Value value: UInt64) {
        self.odwEventProperties.setProperty(name, withUInt64Value: value)
    }

    /**
    Sets an `Integer` property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUInt64Value value: UInt64, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withUInt64Value: value, with: piiKind)
    }

    /**
    Sets a `Bool` property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: A `Bool` that contains the property value.
    */
    public func setProperty(_ name: String, withBoolValue value: Bool) {
        self.odwEventProperties.setProperty(name, withBoolValue: value)
    }

    /**
    Sets a `Bool` property value along with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A `Bool` that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withBoolValue value: Bool, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withBoolValue: value, with: piiKind)
    }

    /**
    Sets a "UUID" property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A "UUID" that contains the property value.
    */
    public func setProperty(_ name: String, withUUIDValue value: UUID) {
        self.odwEventProperties.setProperty(name, withUUIDValue: value)
    }

    /**
    Sets a "UUID" property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A "UUID" that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUUIDValue value: UUID, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withUUIDValue: value, with: piiKind)
    }

    /**
    Sets a date property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A `Date` that contains the property value.
    */
    public func setProperty(_ name: String, withDateValue value: Date) {
        self.odwEventProperties.setProperty(name, withDateValue: value)
    }

    /**
    Sets a date property for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A `Date` that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withDateValue value: Date, withPiiKind piiKind: ODWPiiKind) {
        self.odwEventProperties.setProperty(name, withDateValue: value, with: piiKind)
    }

    /**
    Sets privacy metadata for an event.

    - Parameters:
        - privTags: Privacy data type of the event.
        - privLevel: Privacy diagnostic level of the event.
    */
    public func setPrivacyMetadata(_ privTags: ODWPrivacyDataType, withdWDiagLevel privLevel: ODWDiagLevel) {
        self.odwEventProperties.setProperty("EventInfo.PrivTags", withUInt64Value: UInt64(privTags.rawValue))
        self.odwEventProperties.setProperty("EventInfo.Level", withUInt8Value: UInt8(privLevel.rawValue))
    }
}
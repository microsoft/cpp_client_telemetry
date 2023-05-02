//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/**
Represents Event's properties.
*/
public class EventProperties {
    /// Event Name.
    private var name: String
    /// Event priority.
    private var priority: ODWEventPriority = ODWEventPriority.unspecified
    /// Event properties, Key being the name of the property and Value is property value.
    private var _properties: [String: Any] = [:]
    var properties: [String: Any] {
        get {
            return Dictionary(uniqueKeysWithValues: _properties.lazy.map { ($0, $1) })
        }
    }
    /// Event PII (Personal Identifiable Information) tags, Key being property name and Value is PIIKind value.
    private var _piiTags: [String: ODWPiiKind] = [:]
    var piiTags : [String: ODWPiiKind] {
        get {
            return Dictionary(uniqueKeysWithValues: _piiTags.lazy.map { ($0, $1) })
        }
    }
    /// Base type of an event.
    private var eventType: String = String()

    private func setPiiTag(_ name: String, withPiiKind piiKind: ODWPiiKind) {
         self._piiTags[name] = piiKind
    }

    /**
    Constructs EventProperties object with event name.

    - Parameters:
        - name: Name of the event.
    */
    public init(name: String!) {
        self.name = name
    }

    /**
    Constructs EventProperties object with event name and initial properties.

    - Parameters:
        - name: Name of the event.
        - properties: Initial properties dictionary.
    */
    public init(name: String!, properties: [String: Any]!) {
        self.name = name
        self._properties = properties
    }

    /**
    Constructs EventProperties object with event name, initial properties and PII tags.

    - Parameters:
        - name: Name of the event.
        - properties: Initial properties dictionary.
        - piiTags: Initial PII tags.
    */
    public init(name: String!, withProperties properties: [String: Any]!, withPiiTags piiTags: [String: ODWPiiKind]!) {
        self.name = name
        self._properties = properties
        self._piiTags = piiTags
        self.priority = .unspecified
    }

    /**
    Sets the base type of the event, populated in Records.Type.

    - Parameters:
        - type: Type of the event.
    */
    public func setType(_ type: String) {
        self.eventType = type
    }

    /**
    Sets a string property for an event.

    - Parameters:
        - name: Name of the property
        - value: Value of the property.
    */
    public func setProperty(_ name: String, withValue value: Any) {
        self._properties[name] = value
    }

    /**
    Sets a property for an event with PII tags.

    - Parameters:
        - name: Name of the property.
        - value: Value of the property.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withValue value: Any, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withValue: value)
        self.setPiiTag(name, withPiiKind: piiKind)
    }

    /**
    Sets a double property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A Double that contains the property value.
    */
    public func setProperty(_ name: String, withDoubleValue value: Double) {
        self._properties[name] = value
    }

    /**
    Sets a double property for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A Double that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withDoubleValue value: Double, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withDoubleValue:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets an integer property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withInt64Value value: Int64) {
        self._properties[name] = value
    }

    /**
    Sets an integer property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withInt64Value value: Int64, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withInt64Value:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets an integer property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withUInt8Value value: UInt8) {
        self._properties[name] = value
    }

    /**
    Sets an integer property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUInt8Value value: UInt8, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withUInt8Value:value)
        self.setPiiTag(name, withPiiKind: piiKind)
    }

    /**
    Sets an integer property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
    */
    public func setProperty(_ name: String, withUInt64Value value: UInt64) {
        self._properties[name] = value
    }

    /**
    Sets an integer property value for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: An integer that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUInt64Value value: UInt64, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withUInt64Value:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets a Bool property value for an event.

    - Parameters:
        - name: Name of the property.
        - value: A Bool that contains the property value.
    */
    public func setProperty(_ name: String, withBoolValue value: Bool) {
        self._properties[name] = value
    }

    /**
    Sets a Bool property value along with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A Bool that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withBoolValue value: Bool, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withBoolValue:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets a UUID property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A UUID that contains the property value.
    */
    public func setProperty(_ name: String, withUUIDValue value: UUID) {
        self._properties[name] = value
    }

    /**
    Sets a UUID property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A UUID that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withUUIDValue value: UUID, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withUUIDValue:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets a date property for an event.

    - Parameters:
        - name: Name of the property.
        - value: A Date that contains the property value.
    */
    public func setProperty(_ name: String, withDateValue value: Date) {
        self._properties[name] = value
    }

    /**
    Sets a date property for an event with PiiTags.

    - Parameters:
        - name: Name of the property.
        - value: A Date that contains the property value.
        - piiKind: The kind of Personal Identifieable Information (PII), one from the ::ODWPiiKind enum values.
    */
    public func setProperty(_ name: String, withDateValue value: Date, withPiiKind piiKind: ODWPiiKind) {
        self.setProperty(name, withDateValue:value)
        self.setPiiTag(name, withPiiKind:piiKind)
    }

    /**
    Sets privacy metadat for an event.

    - Parameters:
        - privTags: Privacy data type of the event.
        - privLevel: Privacy diagnostic level of the event.
    */
    public func setPrivacyMetadata(_ privTags: ODWPrivacyDataType, withdWDiagLevel privLevel: ODWDiagLevel) {
        self.setProperty("EventInfo.PrivTags", withUInt64Value:UInt64(privTags.rawValue))
        self.setProperty("EventInfo.Level", withUInt8Value:UInt8(privLevel.rawValue))
    }
}
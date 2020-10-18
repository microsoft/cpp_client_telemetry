//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * <b>Note:</b> These enum values should be in sync with C++ enums defined in EventProperty
 */
enum EventPropertyType {
    TYPE_STRING(0),
    TYPE_LONG(1),
    TYPE_DOUBLE(2),
    TYPE_TIME(3),
    TYPE_BOOLEAN(4),
    TYPE_GUID(5),
    TYPE_STRING_ARRAY(6),
    TYPE_LONG_ARRAY(7),
    TYPE_DOUBLE_ARRAY(8),
    TYPE_GUID_ARRAY(9);

    private final int m_value;

    private EventPropertyType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

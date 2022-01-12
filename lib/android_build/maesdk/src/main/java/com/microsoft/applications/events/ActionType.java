//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The ActionType enumeration contains a set of values that specify the
 * type of action that a user can perform on a page view.
 * They are a general abstraction of action types, each of which corresponds
 * to multiple raw action types. For example, a click action type can be the result of
 * either a button down or a touch tap.
 */
public enum ActionType {
    /**
     * The action type is unspecified.
     */
    Unspecified(0),

    /**
     * The action type is unknown.
     */
    Unknown(1),

    /**
     * The action type is other.
     */
    Other(2),

    /**
     * A mouse click.
     */
    Click(3),

    /**
     * A pan.
     */
    Pan(5),

    /**
     * A zoom.
     */
    Zoom(6),

    /**
     * A hover.
     */
    Hover(7);

    private final int m_value;

    ActionType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}


//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The InputDeviceType enumeration contains a set of values that specify a physical device that a user can use to perform an action on a page view.
 */
public enum InputDeviceType {
    /**
     * Device type unspecified.
     */
    Unspecified(0),

    /**
     * Device type unknown.
     */
    Unknown(1),

    /**
     * Other.
     */
    Other(2),

    /**
     * Mouse.
     */
    Mouse(3),

    /**
     * Keyboard.
     */
    Keyboard(4),

    /**
     * Touch.
     */
    Touch(5),

    /**
     * Stylus.
     */
    Stylus(6),

    /**
     * Microphone.
     */
    Microphone(7),

    /**
     * Kinect.
     */
    Kinect(8),

    /**
     * Camera.
     */
    Camera(9);

    private final int m_value;

    InputDeviceType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}


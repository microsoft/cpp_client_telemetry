//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The RawActionType enumeration contains a set of values that specify the type of physical action that users can perform on a page view.
 */
public enum RawActionType {
    /**
     * Raw action type unspecified.
     */
    Unspecified(0),

    /**
     * Raw action type unknown.
     */
    Unknown(1),

    /**
     * Raw action type other.
     */
    Other(2),

    /**
     * Left button double-click.
     */
    LButtonDoubleClick(11),

    /**
     * Left button down.
     */
    LButtonDown(12),

    /**
     * Left button up.
     */
    LButtonUp(13),

    /**
     * Middle button double-click.
     */
    MButtonDoubleClick(14),

    /**
     * Middle button down.
     */
    MButtonDown(15),

    /**
     * Middle button up.
     */
    MButtonUp(16),

    /**
     * Mouse hover.
     */
    MouseHover(17),

    /**
     * Mouse wheel.
     */
    MouseWheel(18),

    /**
     * Mouse move.
     */
    MouseMove(20),

    /**
     * Right button double-click.
     */
    RButtonDoubleClick(22),

    /**
     * Right button down.
     */
    RButtonDown(23),

    /**
     * Right button up.
     */
    RButtonUp(24),

    /**
     * Touch tap.
     */
    TouchTap(50),

    /**
     * Touch double-tap.
     */
    TouchDoubleTap(51),

    /**
     * Touch long-press.
     */
    TouchLongPress(52),

    /**
     * Touch scroll.
     */
    TouchScroll(53),

    /**
     * Touch pan.
     */
    TouchPan(54),

    /**
     * Touch flick.
     */
    TouchFlick(55),

    /**
     * Touch pinch.
     */
    TouchPinch(56),

    /**
     * Touch zoom.
     */
    TouchZoom(57),

    /**
     * Touch rotate.
     */
    TouchRotate(58),

    /**
     * Keyboard press.
     */
    KeyboardPress(100),

    /**
     * Keyboard Enter.
     */
    KeyboardEnter(101);

    private final int m_value;

    RawActionType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}


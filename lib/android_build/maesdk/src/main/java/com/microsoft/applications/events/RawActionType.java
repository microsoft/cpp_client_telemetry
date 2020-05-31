package com.microsoft.applications.events;

public enum RawActionType {
    /// <summary>Raw action type unspecified.</summary>
    Unspecified(0),
    /// <summary>Raw action type unknown.</summary>
    Unknown(1),
    /// <summary>Raw action type other.</summary>
    Other(2),
    /// <summary>Left button double-click.</summary>
    LButtonDoubleClick(11),
    /// <summary>Left button down.</summary>
    LButtonDown(12),
    /// <summary>Left button up.</summary>
    LButtonUp(13),
    /// <summary>Middle button double-click.</summary>
    MButtonDoubleClick(14),
    /// <summary>Middle button down.</summary>
    MButtonDown(15),
    /// <summary>Middle button up.</summary>
    MButtonUp(16),
    /// <summary>Mouse hover.</summary>
    MouseHover(17),
    /// <summary>Mouse wheel.</summary>
    MouseWheel(18),
    /// <summary>Mouse move.</summary>
    MouseMove(20),
    /// <summary>Right button double-click.</summary>
    RButtonDoubleClick(22),
    /// <summary>Right button down.</summary>
    RButtonDown(23),
    /// <summary>Right button up.</summary>
    RButtonUp(24),
    /// <summary>Touch tap.</summary>
    TouchTap(50),
    /// <summary>Touch double-tap.</summary>
    TouchDoubleTap(51),
    /// <summary>Touch long-press.</summary>
    TouchLongPress(52),
    /// <summary>Touch scroll.</summary>
    TouchScroll(53),
    /// <summary>Touch pan.</summary>
    TouchPan(54),
    /// <summary>Touch flick.</summary>
    TouchFlick(55),
    /// <summary>Touch pinch.</summary>
    TouchPinch(56),
    /// <summary>Touch zoom.</summary>
    TouchZoom(57),
    /// <summary>Touch rotate.</summary>
    TouchRotate(58),
    /// <summary>Keyboard press.</summary>
    KeyboardPress(100),
    /// <summary>Keyboard Enter.</summary>
    KeyboardEnter(101);

    private final int m_value;

    private RawActionType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

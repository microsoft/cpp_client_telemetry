package com.microsoft.applications.events;

public enum InputDeviceType {
    /// <summary>Device type unspecified.</summary>
    Unspecified(0),
    /// <summary>Device type unknown.</summary>
    Unknown(1),
    /// <summary>Other.</summary>
    Other(2),
    /// <summary>Mouse.</summary>
    Mouse(3),
    /// <summary>Keyboard.</summary>
    Keyboard(4),
    /// <summary>Touch.</summary>
    Touch(5),
    /// <summary>Stylus.</summary>
    Stylus(6),
    /// <summary>Microphone.</summary>
    Microphone(7),
    /// <summary>Kinect.</summary>
    Kinect(8),
    /// <summary>Camera.</summary>
    Camera(9);

    private final int m_value;

    private InputDeviceType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

package com.microsoft.applications.events;

import java.util.UUID;

class Logger implements ILogger {
    private final long m_nativePtr;

    Logger(long nativePtr) {
        m_nativePtr = nativePtr;
    }

    private native long getSemanticContext(long nativePtr);

    public ISemanticContext getSemanticContext() {
        return new ISemanticContext(getSemanticContext(m_nativePtr));
    }

    private static native int nativeSetContextStringValue(long nativePtr, String name, String value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final String value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextStringValue(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     */
    public void setContext(final String name, final String value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextDoubleValue(long nativePtr, String name, double value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final double value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextDoubleValue(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     */
    public void setContext(final String name, final double value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextLongValue(long nativePtr, String name, long value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A long that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final long value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextLongValue(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A long that contains the property value.
     */
    public void setContext(final String name, final long value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextIntValue(long nativePtr, String name, int value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(String name, final int value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextIntValue(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     */
    public void setContext(String name, final int value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextBoolValue(long nativePtr, String name, boolean value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final boolean value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextBoolValue(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     */
    public void setContext(final String name, final boolean value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextTimeTicksValue(long nativePtr, String name, long value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a .NET time_ticks_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A .NET time_ticks_t that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final time_ticks_t value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextTimeTicksValue(m_nativePtr, name, value.m_ticks, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a .NET time_ticks_t that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A .NET time_ticks_t that contains the property value.
     */
    public void setContext(final String name, final time_ticks_t value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private static native int nativeSetContextGuidValue(long nativePtr, String name, String value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final UUID value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        nativeSetContextGuidValue(m_nativePtr, name, value.toString(), piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     */
    public void setContext(final String name, final UUID value) {
        setContext(name, value, PiiKind.PiiKind_None);
    }

    private native void nativeSetContextEventProperty(long nativePtr, String name, EventProperty prop);

    /**
     * Populate event property using EventProperty value object.
     * @param name Property name.
     * @param prop Property value object.
     */
    public void SetContext(final String name, final EventProperty prop) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (prop == null)
            throw new IllegalArgumentException("prop is null");

        nativeSetContextEventProperty(m_nativePtr, name, prop);
    }
}

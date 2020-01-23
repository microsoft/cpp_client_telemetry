package com.microsoft.applications.events;

public class ILogger {
    private final long m_nativePtr;

    ILogger(long nativePtr) {
        m_nativePtr = nativePtr;
    }

    private native void nativeSetContext(long nativePtr, String name, String value, int piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property
     * @param value A string that contains the property value
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final String value, final PiiKind piiKind) {
        nativeSetContext(m_nativePtr, name, value, piiKind.getValue());
    }

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property
     * @param value A string that contains the property value
     */
    public void setContext(final String name, final String value){
        setContext(name, value, PiiKind.PiiKind_None);
    }
}

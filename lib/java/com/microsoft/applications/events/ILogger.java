package com.microsoft.applications.events;

import java.util.Date;
import java.util.UUID;

public interface ILogger {

    /**
     * Gets an ISemanticContext interface through which you can specify the semantic context for this logger instance.
     *
     * @return An instance of the ISemanticContext interface
     */
    public ISemanticContext getSemanticContext();

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final String value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     */
    public void setContext(final String name, final String value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final double value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     */
    public void setContext(final String name, final double value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     *
     * @param value A long that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final long value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A long that contains the property value.
     */
    public void setContext(final String name, final long value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(String name, final int value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     */
    public void setContext(String name, final int value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final boolean value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     */
    public void setContext(final String name, final boolean value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a Date that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value The property's Date value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final Date value, PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a Date that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value The property's Date value.
     */
    public void setContext(final String name, final Date value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final UUID value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     */
    public void setContext(final String name, final UUID value);

    /**
     * Populate event property using EventProperty value object.
     *
     * @param name Property name.
     * @param prop Property value object.
     */
    public void SetContext(final String name, final EventProperty prop);
}

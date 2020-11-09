//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;
import java.util.Date;
import java.util.UUID;

public class EventProperty {

    private PiiKind m_piiKind;
    private DataCategory m_category = DataCategory.PartC;
    private EventPropertyValue m_eventPropertyValue;

    public PiiKind getPiiKind() {
        return m_piiKind;
    }

    @Keep
    int getPiiKindValue() {
        return m_piiKind.getValue();
    }

    @Keep
    int getDataCategoryValue() {
        return m_category.getValue();
    }

    DataCategory getDataCategory() {
        return m_category;
    }

    @Keep
    public EventPropertyValue getEventPropertyValue() {
        return m_eventPropertyValue;
    }

    /**
     * The EventProperty constructor, taking a string.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A string.
     */
    public EventProperty(final String value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a string.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A string.
     */
    public EventProperty(final String value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a string,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A string.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final String value, final PiiKind piiKind, final DataCategory category) {
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyStringValue(value);
    }

    /**
     * The EventProperty constructor, taking an int.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value An int.
     */
    public EventProperty(final int value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a int.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A int.
     */
    public EventProperty(final int value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking an int,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value An int.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final int value, final PiiKind piiKind, final DataCategory category) {
        this((long)value, piiKind, category);
    }

    /**
     * The EventProperty constructor, taking a long.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A long.
     */
    public EventProperty(final long value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a long.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A long.
     */
    public EventProperty(final long value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a long,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A long.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final long value, final PiiKind piiKind, final DataCategory category) {
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyLongValue(value);
    }

    /**
     * The EventProperty constructor, taking a double.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A double.
     */
    public EventProperty(final double value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a double.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A double.
     */
    public EventProperty(final double value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a double,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A double.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final double value, final PiiKind piiKind, final DataCategory category) {
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyDoubleValue(value);
    }

    /**
     * The EventProperty constructor, taking a boolean.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A boolean.
     */
    public EventProperty(final boolean value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a boolean.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A boolean.
     */
    public EventProperty(final boolean value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a boolean,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A boolean.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final boolean value, final PiiKind piiKind, final DataCategory category) {
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyBooleanValue(value);
    }

    /**
     * The EventProperty constructor, taking a Date.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A Date.
     */
    public EventProperty(final Date value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a Date.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A Date.
     */
    public EventProperty(final Date value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a Date,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A Date.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final Date value, final PiiKind piiKind, final DataCategory category) {
        this(new TimeTicks(value), piiKind, category);
    }

    /**
     * The EventProperty constructor, taking a TimeTicks,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A string.
     * @param piiKind The kind of personal identifiable information.
     * @param category TimeTicks of the event
     */
    private EventProperty(final TimeTicks value, final PiiKind piiKind, final DataCategory category) {
        if (value == null)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyTimeTicksValue(value.getTicks());
    }

    /**
     * The EventProperty constructor, taking a UUID/GUID.
     * Defaults to the the kind of personal identifiable information PiiKind.None
     * and tags the property with default DataCategory.PartC.
     *
     * @param value A UUID.
     */
    public EventProperty(final UUID value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking a UUID/GUID.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value A UUID.
     */
    public EventProperty(final UUID value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking a UUID/GUID,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value A UUID.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final UUID value, final PiiKind piiKind, final DataCategory category) {
        if (value == null)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyGuidValue(value);
    }

    /**
     * The EventProperty constructor, taking an array of strings.
     * Defaults to the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param value An array of strings.
     */
    public EventProperty(final String[] value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking an array of string.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value An array of string.
     */
    public EventProperty(final String[] value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking an array of strings,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value An array of strings.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final String[] value, final PiiKind piiKind, final DataCategory category) {
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyStringArrayValue(value);
    }

    /**
     * The EventProperty constructor, taking an array of long.
     * Defaults to the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param value An array of long.
     */
    public EventProperty(final long[] value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking an array of long.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value An array of long.
     */
    public EventProperty(final long[] value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking an array of long,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value An array of long.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final long[] value, final PiiKind piiKind, final DataCategory category) {
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyLongArrayValue(value);
    }

    /**
     * The EventProperty constructor, taking an array of double.
     * Defaults to the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param value An array of double.
     */
    public EventProperty(final double[] value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking an array of double.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value An array of double.
     */
    public EventProperty(final double[] value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking an array of double,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value An array of double.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final double[] value, final PiiKind piiKind, final DataCategory category) {
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyDoubleArrayValue(value);
    }

    /**
     * The EventProperty constructor, taking an array of UUID/GUID.
     * Defaults to the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param value An array of UUID.
     */
    public EventProperty(final UUID[] value) {
        this(value, PiiKind.None);
    }

    /**
     * The EventProperty constructor, taking an array of UUID/GUID.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param value An array of UUID.
     */
    public EventProperty(final UUID[] value, final PiiKind piiKind) {
        this(value, piiKind, DataCategory.PartC);
    }

    /**
     * The EventProperty constructor, taking an array of UUID/GUID,
     * and the kind of personal identifiable information, and the DataCategory for the event.
     *
     * @param value An array of UUID.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final UUID[] value, final PiiKind piiKind, final DataCategory category) {
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyGuidArrayValue(value);
    }
}


package com.microsoft.applications.events;

import android.provider.ContactsContract;

import java.time.chrono.ChronoZonedDateTime;
import java.util.UUID;

public class EventProperty {

    private PiiKind m_piiKind;
    private DataCategory m_category = DataCategory.DataCategory_PartC;
    private EventPropertyValue m_eventPropertyValue;

    PiiKind getPiiKind() {
        return m_piiKind;
    }

    DataCategory getDataCategory() {
        return m_category;
    }

    public EventPropertyValue getEventPropertyValue() {
        return m_eventPropertyValue;
    }

    /**
     * The EventProperty constructor, taking a string, and the kind of personal identifiable information.
     * and tags the property with default DataCategory_PartC.
     * @param value A string.
     */
    public EventProperty(final String value) {
        this(value, PiiKind.PiiKind_None, DataCategory.DataCategory_PartC);
    }

    /**
     * The EventProperty constructor, taking a string, and the kind of personal identifiable information.
     * @param value A string.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(final String value, final PiiKind piiKind, final DataCategory category) {
        if (value == null || value.trim().isEmpty())
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        m_piiKind = piiKind;
        m_category = category;
        m_eventPropertyValue = new EventPropertyStringValue(value);
    }

    //public EventProperty(final long value) {
    //    eventPropertyValue = new EventPropertyLongValue(value);
    //}
    //
    //public EventProperty(final double value) {
    //    eventPropertyValue = new EventPropertyDoubleValue(value);
    //}
    //
    //public EventProperty(final boolean value) {
    //    eventPropertyValue = new EventPropertyBooleanValue(value);
    //}
    //
    //public EventProperty(final time_ticks_t value) {
    //    eventPropertyValue = new EventPropertyTimeTicksValue(value);
    //}
    //
    //public EventProperty(final UUID value) {
    //    eventPropertyValue = new EventPropertyGuidValue(value.toString());
    //}

}

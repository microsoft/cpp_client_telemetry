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

    EventPropertyValue getEventPropertyValue() {
        return m_eventPropertyValue;
    }

    /**
     * The EventProperty constructor, taking a string, and the kind of personal identifiable information.
     * @param value A string.
     */    public EventProperty(final String value) {
        this(value, PiiKind.PiiKind_None, DataCategory.DataCategory_PartC);
    }

    /**
     * The EventProperty constructor, taking a string, and the kind of personal identifiable information.
     * @param value A string.
     * @param piiKind The kind of personal identifiable information.
     * @param category DataCategory of the event
     */
    public EventProperty(String value, PiiKind piiKind, DataCategory category) {
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
    //public EventProperty(final ChronoZonedDateTime value) {
    //    eventPropertyValue = new EventPropertyTimeTicksValue(value);
    //}
    //
    //public EventProperty(final UUID value) {
    //    eventPropertyValue = new EventPropertyGuidValue(value.toString());
    //}

}

package com.microsoft.applications.events;

import java.util.Date;
import java.util.Map;
import java.util.UUID;

public class EventProperties {
    private static final String DEFAULT_EVENT_NAME = "undefined";
    private EventPropertiesStorage mStorage;

    /**
     * Gets the internal storage used for EventProperties.
     *
     * @return EventPropertiesStorage
     */
    EventPropertiesStorage getStorage() {
        return mStorage;
    }

    /**
     * Constructs an EventProperties object, taking a string for the property name and a diagnostic level.
     * You must supply a non-empty name whenever you supply any custom properties for the event via EventProperties.
     * 
     * @param name Name for the EventProperties
     * @param diagnosticLevel Diagnostic level defined in com.microsoft.applications.events.Constants
     */
    public EventProperties(final String name, int diagnosticLevel) {
        if (name == null || !Utils.validatePropertyName(name))
            throw new IllegalArgumentException("name is null or invalid");

        if (!setName(name))
            throw new IllegalArgumentException("name is invalid");

        setLevel(diagnosticLevel);
        mStorage = new EventPropertiesStorage();
    }

    /**
     * Constructs an EventProperties object, taking a string for the property name.
     * Sets the diagnostic level of the event to DIAG_LEVEL_OPTIONAL
     * You must supply a non-empty name whenever you supply any custom properties for the event via EventProperties.
     *
     * @param name Name for the EventProperties
     */
    public EventProperties(final String name) {
        this(name, Constants.DIAG_LEVEL_OPTIONAL);
    }

    /**
     * Constructs an EventProperties object (the default constructor).
     * Sets the diagnostic level of the event to DIAG_LEVEL_OPTIONAL
     * Default event name set to "undefined", you should set an appropriate event name.
     */
    public EventProperties() {
        this(DEFAULT_EVENT_NAME);
    }

    /**
     * The EventProperties copy constructor.
     */
    public EventProperties(final EventProperties copy) {
        if (copy == null)
            throw new IllegalArgumentException("copy is null");

        mStorage = new EventPropertiesStorage(copy.mStorage);
    }

    /**
     * Constructs an EventProperties object from a map of string to EventProperty.
     * Sets the diagnostic level of the event to DIAG_LEVEL_OPTIONAL
     * You must supply a non-empty name whenever you supply any custom properties for the event via EventProperties.
     *
     * @param name Name for the EventProperties
     * @param properties Properties to be added to the event
     */
    public EventProperties(final String name, final Map<String, EventProperty> properties) {
        this(name);
        addProperties(properties);
    }

    /**
     * Adds a map of <string, EventProperty> to EventProperties.
     *
     * @param properties map of <string, EventProperty>
     */
    public void addProperties(final Map<String, EventProperty> properties) {
        mStorage.addProperties(properties);
    }

    /**
     * Assigns a map of <string, EventProperty> to EventProperties.
     * Any previous existing properties would be removed.
     *
     * @param properties map of <string, EventProperty>
     */
    public void assignProperties(final Map<String, EventProperty> properties) {
        mStorage.assignProperties(properties);
    }

    /**
     * Sets the name of an event, given a string for the event name.
     * You must supply a non-empty name whenever you supply any custom properties for the event via <b>EventProperties</b>.
     *
     * @param name A string that contains the name of the event.
     * @return true if succeed or false if failed.
     */
    public boolean setName(final String name) {
        if (name != null && Utils.validateEventName(name)) {
            mStorage.eventName = name;
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * Gets the name of an event. An empty string is returned if the name was never set.
     *
     * @return Name of the event
     */
    public String getName() {
        return mStorage.eventName;
    }

    /**
     * Sets the base type of an event.
     *
     * @param recordType Base Type of event record.
     * @return true if succeed or false if failed.
     */
    public boolean setType(final String recordType) {
        if (recordType != null && Utils.validateEventName(recordType)) {
            mStorage.eventType = recordType;
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * Gets the Base Type of an event.
     * An empty string is returned if this was never set.
     *
     * @return A string that contains the type of the event.
     */
    public String getType() {
        return mStorage.eventType;
    }

    /**
     * [optional] Sets the timestamp of an event, in milliseconds.
     * This method overrides the default timestamp generated by the telemetry system.
     *
     * @param timestampInEpochMillis The UNIX timestamp in milliseconds. This is the amount of time since 00:00:00
     * Coordinated Universal Time (UTC), January, 1, 1970 (not counting leap seconds).
     */
    public void setTimestamp(final long timestampInEpochMillis) {
        mStorage.timestampInMillis = timestampInEpochMillis;
    }

    /**
     * Gets the timestamp of an event, in milliseconds.
     * Zero is returned when the time stamp was not specified with SetTimestamp().
     *
     * @return The timestamp of the event, specified in milliseconds.
     */
    public long getTimestamp() {
        return mStorage.timestampInMillis;
    }

    /**
     * [optional] Sets the transmit priority of an event.
     * If you don't specify a value, then the default priority is used.
     *
     * @param priority The transmit priority.
     */
    public void setPriority(EventPriority priority) {
        mStorage.eventLatency = EventLatency.getEnum(priority.getValue());
        if (priority.getValue() >= EventPriority.High.getValue())
        {
            mStorage.eventLatency = EventLatency.RealTime;
            mStorage.eventPersistence = EventPersistence.Critical;
        }
        else if (priority.getValue() >= EventPriority.Low.getValue())
        {
            mStorage.eventLatency = EventLatency.Normal;
            mStorage.eventPersistence = EventPersistence.Normal;
        }
    }

    /**
     * Gets the transmit priority of the event.
     * If you don't specify a value, then the default priority is used.
     *
     * @return The transmit priority.
     */
    public EventPriority getPriority() {
        return EventPriority.getEnum(mStorage.eventLatency.getValue());
    }

    /**
     * [optional] Sets the transmit Latency of the event.
     * Default transmit Latency will be used if none specified.
     *
     * @param latency Event latency.
     */
    public void setLatency(EventLatency latency) {
        mStorage.eventLatency = latency;
    }

    /**
     * Get the transmit Latency of the event.
     * Default transmit Latency will be used if none specified.
     *
     * @return Transmit Latency of the event
     */
    public EventLatency getLatency() {
        return mStorage.eventLatency;
    }

    /**
     * [optional] Specify Persistence priority of an event.
     * Default Persistence priority will be used for persisting the event if none was specified.
     *
     * @param persistence Persistence of the event
     */
    public void setPersistence(EventPersistence persistence) {
        mStorage.eventPersistence = persistence;
    }

    /**
     * Get Persistence of this event.
     * Default Persistence priority will be used for persisting the event if none was specified.
     *
     * @return Persistence of the event
     */
    public EventPersistence getPersistence() {
        return mStorage.eventPersistence;
    }

    /**
     * [optional] Specify popSample of an event.
     *
     * @param popSample of the event
     */
    public void setPopSample(double popSample) {
        mStorage.eventPopSample = popSample;
    }

    /**
     * Get the popSample of the event.
     *
     * @return popSample of the event
     */
    public double getPopSample() {
        return mStorage.eventPopSample;
    }

    /**
     * [optional] Specify Policy Bit flags for UTC usage of an event.
     * Default values will be used for transmitting the event if none was specified.
     *
     * @param policyBitFlags of the event
     */
    public void setPolicyBitFlags(long policyBitFlags) {
        mStorage.eventPolicyBitflags = policyBitFlags;
    }

    /**
     * Get the Policy bit flags for UTC usage of the event.
     *
     * @return Policy bit flags of the event
     */
    public long getPolicyBitFlags() {
        return mStorage.eventPolicyBitflags;
    }

    /**
     * Sets the diagnostic level of an event. This is equivalent to:
     * SetProperty(Constants.COMMONFIELDS_EVENT_LEVEL, level);
     *
     * @param level Diagnostic level of the event.
     */
    public void setLevel(int level) {
        setProperty(Constants.COMMONFIELDS_EVENT_LEVEL, level);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property
     * @param prop Value of the property
     */
    public void setProperty(final String name, final EventProperty prop) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if(!Utils.validatePropertyName(name))
            throw new IllegalArgumentException("name:" + name  + " is invalid.");
        if (prop == null)
            throw new IllegalArgumentException("prop is null");

        mStorage.properties.put(name, prop);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final String value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final String value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    public void setProperty(final String name, final String value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final double value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final double value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final double value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final int value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final int value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final int value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final long value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final long value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final long value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final boolean value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final boolean value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final boolean value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final Date value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final Date value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final Date value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final UUID value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final UUID value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final UUID value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final String[] value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final String[] value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final String[] value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final UUID[] value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final UUID[] value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final UUID[] value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final double[] value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final double[] value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final double[] value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * Defaults to the the kind of personal identifiable information PiiKind.None,
     * and tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     */
    public void setProperty(final String name, final long[] value) {
        setProperty(name, value, PiiKind.None);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     * By default tags the property with default DataCategory.PartC.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property.
     */
    public void setProperty(final String name, final long[] value, final PiiKind piiKind) {
        setProperty(name, value, piiKind, DataCategory.PartC);
    }

    /**
     * Specify a property for an event.
     * It either creates a new property if none exists or overwrites the existing one.
     *
     * @param name Name of the property.
     * @param value Value of the property.
     * @param piiKind PIIKind of the property, if passed null default PiiKind.None is set.
     * @param category DataCategory of the property, if passed null default DataCategory.PartC is set.
     */
    void setProperty(final String name, final long[] value, final PiiKind piiKind, final DataCategory category) {
        setProperty(name, new EventProperty(value, piiKind, category));
    }

    /**
     * Get the properties bag of an event for default DataCategory.PartC
     *
     * @return Properties bag of the event
     */
    public Map<String, EventProperty> getProperties() {
        return getProperties(DataCategory.PartC);
    }

    /**
     * Get the properties bag of an event.
     *
     * @param category DataCategory of the properties
     * @return Properties bag of the event
     */
    public Map<String, EventProperty> getProperties(final DataCategory category) {
        if (category == DataCategory.PartC)
            return mStorage.properties;
        else
            return mStorage.propertiesPartB;
    }

    /**
     * Erase property from event for default DataCategory.PartC
     *
     * @param key Property key to be deleted
     * @return boolean value true if key found or else false
     */
    boolean erase(final String key) {
        return erase(key, DataCategory.PartC);
    }

    /**
     * Erase property from event.
     *
     * @param key Property key to be deleted
     * @param category DataCategory of the event properties
     * @return boolean value true if key found or else false
     */
    boolean erase(final String key, final DataCategory category) {
        if (key == null || key.trim().isEmpty())
            throw new IllegalArgumentException("key is null or empty");
        if (category == null)
            throw new IllegalArgumentException("category is null");

        Map<String, EventProperty> props = category == DataCategory.PartC ? mStorage.properties : mStorage.propertiesPartB;
        EventProperty prop = props.remove(key);
        return prop == null;
    }
}

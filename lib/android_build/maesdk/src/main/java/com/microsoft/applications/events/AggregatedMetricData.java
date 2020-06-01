package com.microsoft.applications.events;

import java.util.Map;

/**
 * The AggregatedMetricData structure contains the data of a precomputed aggregated metrics event.
 */
public class AggregatedMetricData {
    /**
     * [Required] The name of the precomputed aggregated metric.
     */
    public String name;

    /**
     * [Required] The duration (length of time in microseconds) that this aggregated metric spans.
     */
    public long duration;

    /**
     * [Required] The total count of metric observations aggregated in the duration.
     */
    public long count;

    /**
     * [Optional] A string representing the units of measure of the aggregated metric.
     */
    public String units;

    /**
     * [Optional] An instance name for the aggregated metric.
     */
    public String instanceName;

    /**
     * [Optional] A string that contains the object class upon which the aggregated metric is measured.
     */
    public String objectClass;

    /**
     * [Optional] A string that contains the object ID upon which the Aggregated Metric is measured.
     */
    public String objectId;

    /**
     * [Optional] The reported aggregated metrics.
     * The types of aggregates are specified by the ::AggregateType enumeration.
     */
    public Map<AggregateType, Double> aggregates;

    /**
     * [Optional] A standard map that contains a frequency table, which is an alternative way to summarize the observations (like a time series).
     */
    public Map<Long, Long> buckets;

    /**
     * An AggregatedMetricData constructor
     * that takes a string that contains the name of the aggregated metric,
     * a long that contains the duration of the aggregation,
     * and a long that contains the count of the number of occurrences.
     *
     * @param aggrName Name of the aggregated metric
     * @param aggrDuration Duration of the aggregation
     * @param aggrCount Number of occurrences
     */
    AggregatedMetricData(final String aggrName, final long aggrDuration, final long aggrCount) {
        name = aggrName;
        duration = aggrDuration;
        count = aggrCount;
    }
}

package com.microsoft.applications.events;

import java.util.Map;

public class AggregatedMetricData {
    /// <summary>
    /// [Required] The name of the precomputed aggregated metric.
    /// </summary>
    public String name;

    /// <summary>
    /// [Required] The duration (length of time in microseconds) that this aggregated metric spans.
    /// </summary>
    public long duration;

    /// <summary>
    /// [Required] The total count of metric observations aggregated in the duration.
    /// </summary>
    public long count;

    /// <summary>
    /// [Optional] A string representing the units of measure of the aggregated metric.
    /// </summary>
    public String units;

    /// <summary>
    /// [Optional] An instance name for the aggregated metric.
    /// </summary>
    public String instanceName;

    /// <summary>m
    /// [Optional] A string that contains the object class upon which the aggregated metric is measured.
    /// </summary>
    public String objectClass;

    /// <summary>
    /// [Optional] A string that contains the object ID upon which the Aggregated Metric is measured.
    /// </summary>
    public String objectId;

    /// <summary>
    /// [Optional] The reported aggregated metrics.
    /// The types of aggregates are specified by the ::AggregateType enumeration.
    /// </summary>
    public Map<AggregateType, Double> aggregates;

    /// <summary>
    /// [Optional] A standard map that contains a frequency table, which is an alternative way to summarize the observations (like a time series).
    /// </summary>
    public Map<Long, Long> buckets;

    /// <summary>
    /// An AggregatedMetricData constructor
    /// that takes a string that contains the name of the aggregated metric,
    /// a long that contains the duration of the aggregation,
    /// and a long that contains the count of the number of occurrences.
    /// </summary>
    /// <param name='aggrName'>Name of the aggregated metric</param>
    /// <param name='aggrDuration'>Duration of the aggregation</param>
    /// <param name='aggrCount'>Number of occurrences</param>
    AggregatedMetricData(final String aggrName, final long aggrDuration, final long aggrCount) {
        name = aggrName;
        duration = aggrDuration;
        count = aggrCount;
    }
}

// Copyright (c) Microsoft. All rights reserved.
#ifndef ARIA_ILOGGER_HPP
#define ARIA_ILOGGER_HPP

//#include "Version.hpp"
#include "ctmacros.hpp"
#include "Enums.hpp"
#include "EventProperties.hpp"
#include "ISemanticContext.hpp"
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*

//Data Type Flags
#define MICROSOFT_KEYWORD_CRITICAL_DATA 0x0000800000000000 // Bit 47
#define MICROSOFT_KEYWORD_MEASURES      0x0000400000000000 // Bit 46
#define MICROSOFT_KEYWORD_TELEMETRY     0x0000200000000000 // Bit 45
//Core data Flags
#define MICROSOFT_EVENTTAG_CORE_DATA            0x00080000
//Latency Flags
#define MICROSOFT_EVENTTAG_COSTDEFERRED_LATENCY 0x00040000
#define MICROSOFT_EVENTTAG_REALTIME_LATENCY     0x00200000
#define MICROSOFT_EVENTTAG_NORMAL_LATENCY       0x00400000

/// <summary>
/// Struct data of a page action event.
/// </summary>
struct PageActionData
{
    /// [Required] ID of the page view with which this action is associated
    std::string pageViewId;

    /// [Required] Generic abstraction of the type of the action
    ActionType actionType;

    /// [Optional] the type of Physical action
    RawActionType rawActionType;

    /// [Optional] the type of input device that generates this action
    InputDeviceType inputDeviceType;

    /// [Optional] ID of the item on which this action acts
    std::string targetItemId;

    /// [Optional] Name of the data source item on which this action acts
    std::string targetItemDataSourceName;

    /// [Optional] Name of the data source category the item belongs to
    std::string targetItemDataSourceCategory;

    /// [Optional] Name of the data source colletion the item belongs to
    std::string targetItemDataSourceCollection;

    /// [Optional] Name of the layout container to which the item belongs
    std::string targetItemLayoutContainer;

    /// [Optional] Relative ordering/ranking/positioning within the layout container item has
    unsigned short targetItemLayoutRank;

    /// [Optional] Destination Uri resulted by this action
    std::string destinationUri;

    PageActionData(std::string const& pvId, ActionType actType)
      : pageViewId(pvId),
        actionType(actType),
        rawActionType(RawActionType_Unspecified),
        inputDeviceType(InputDeviceType_Unspecified),
        targetItemLayoutRank(0)
    {}
};

/// <summary>
/// Struct data of an pre-computed aggregated metrics event
/// </summary>
struct AggregatedMetricData
{
    /// [Required] Name of the pre-computed aggregated metric
    std::string name;

    /// [Required] Duration in microseconds this aggregated metric covers.
    long duration;

    /// [Required] Total count of metric observations aggregared in the duration.
    long count;

    /// [Optional] String representing units of the aggregated metric.
    std::string units;

    /// [Optional] An instance name for the Aggregated Metric.
    std::string instanceName;

    /// [Optional] String indicating the object class on which the Aggregated Metric is being measured.
    std::string objectClass;

    /// [Optional] string indicating the object id on which the Aggregated Metric is being measured.
    std::string objectId;

    /// [Optional] aggregated metrics being reported.
    /// The types of aggregates are specified by the AggregateType enum
    std::map<AggregateType, double> aggregates;

    /// [Optional] Frequency table is an optional way to report summary of the observations like time series.
    std::map<long, long> buckets;

    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name='aggrName'>Name of the aggregated metric</param>
    /// <param name='aggrDuration'>Duration of the aggregation</param>
    /// <param name='aggrCount'>Number of occurrences</param>
    AggregatedMetricData(std::string const& aggrName, long aggrDuration, long aggrCount)
      : name(aggrName),
        duration(aggrDuration),
        count(aggrCount)
    {}
};

/// <summary>
/// ILogger interface for logging either semantic or custom event
/// </summary>
class ARIASDK_LIBABI ILogger
{

  public:
    virtual ~ILogger() {}
    /// <summary>
    /// Gets an ISemanticContext interface through which to specify semantic context of this logger instance.
    /// </summary>
    /// <returns>ISemanticContext interface</returns>
    virtual ISemanticContext* GetSemanticContext() const = 0 ;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, const char value[], PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">Double value of the property</param>
	virtual void SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">64-bit Integer value of the property</param>
	virtual void SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">GUID</param>
	virtual void SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) = 0;

    /// <summary>
    /// Logs a state of application lifecycle into which the application has transitioned.
    /// </summary>
    /// <param name="state">state in the application's lifecycle</param>
    /// <param name="properties">Properties of this AppLifecycle event</param>
    virtual void LogAppLifecycle(AppLifecycleState state, EventProperties const& properties) = 0;

	/// <summary>
	/// Logs a state of application Session into which the application has transitioned. 
	/// </summary>
	/// <param name="state">state in the application's lifecycle</param>
	/// <param name="properties">Properties of this Session event</param>
	virtual void LogSession(SessionState state, const EventProperties& properties) = 0;

    /// <summary>
    /// Logs the custom event with the specified name
    /// </summary>
    /// <param name="name">Name of the custom event</param>
    virtual void LogEvent(std::string const& name) = 0;

    /// <summary>
    /// Logs a custom event with specified name and properties to track information
    /// such as how a particular feature is used.
    /// </summary>
    /// <param name="properties">Properties of the custom event</param>
    virtual void LogEvent(EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a failure event such as application exception.
    /// </summary>
    /// <param name="signature">Signature string that identifies the bucket of the failure</param>
    /// <param name="detail">Detail of the failure</param>
    /// <param name="properties">Properties of the failure event</param>
    virtual void LogFailure(std::string const& signature,
        std::string const& detail,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a failure event such as application exception.
    /// </summary>
    /// <param name="signature">Signature string that identifies the bucket of the failure</param>
    /// <param name="detail">Detail of the failure</param>
    /// <param name="category">Category of the failure such as application error, not responding or crash</param>
    /// <param name="id">Identifier that could be used to uniquely identify or reference this failure</param>
    /// <param name="properties">Properties of the failure event</param>
    virtual void LogFailure(std::string const& signature,
        std::string const& detail,
        std::string const& category,
        std::string const& id,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a page view event which is normally a result of a user action on a UI page
    /// such as search query, content request or page navigation.
    /// </summary>
    /// <param name="id">Identifier that could be used to uniquely identify or reference this page</param>
    /// <param name="pageName">Friendly name of the page</param>
    /// <param name="properties">Properties of the page view event</param>
    virtual void LogPageView(std::string const& id,
        std::string const& pageName,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a page view event which is normally a result of a user action on a UI page
    /// such as search query, content request or page navigation.
    /// </summary>
    /// <param name="id">Identifier that could be used to uniquely identify or reference this page</param>
    /// <param name="pageName">Friendly name of the page</param>
    /// <param name="category">Category to which this page belongs</param>
    /// <param name="uri">Uri of this page</param>
    /// <param name="referrerUri">Uri that refers to this page</param>
    /// <param name="properties">Properties of the page view event</param>
    virtual void LogPageView(std::string const& id,
        std::string const& pageName,
        std::string const& category,
        std::string const& uri,
        std::string const& referrerUri,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a page action event.
    /// </summary>
    /// <param name="pageViewId">identifier that uniquely identifies the page view on which this action occurs</param>
    /// <param name="actionType">Generic type of the page action</param>
    /// <param name="properties">Properties of the page action event</param>
    virtual void LogPageAction(std::string const& pageViewId,
        ActionType actionType,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a detailed page action event.
    /// </summary>
    /// <param name="pageActionData">Detailed information about the page action </param>
    /// <param name="properties">Properties of the page action event</param>
    virtual void LogPageAction(PageActionData const& pageActionData,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a sampled metric event such as a performance counter.
    /// </summary>
    /// <param name="name">Name of the sampled metric</param>
    /// <param name="value">Value of the sampled metric</param>
    /// <param name="units">String representation for the units of the metric value</param>
    /// <param name="properties">Properties of the sampled metric event</param>
    virtual void LogSampledMetric(std::string const& name,
        double value,
        std::string const& units,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a sampled metric event such as a performance counter.
    /// </summary>
    /// <param name="name">Name of the sampled metric</param>
    /// <param name="value">Value of the sampled metric</param>
    /// <param name="units">String representation for the units of the metric value</param>
    /// <param name="instanceName">Name of this metric instance e.g. performance counter</param>
    /// <param name="objectClass">Object class for which this metric is tracking</param>
    /// <param name="objectId">Object ID for which this metric is tracking</param>
    /// <param name="properties">Properties of this sampled metric event</param>
    virtual void LogSampledMetric(std::string const& name,
        double value,
        std::string const& units,
        std::string const& instanceName,
        std::string const& objectClass,
        std::string const& objectId,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a pre-computed aggregated metric event e.g. queue length.
    /// </summary>
    /// <param name="name">Name of the aggregated metric</param>
    /// <param name="duration">Duration in microseconds that this metric are aggregated</param>
    /// <param name="count">Count of the aggregated metric observations</param>
    /// <param name="properties">Properties of this aggregated metric event</param>
    virtual void LogAggregatedMetric(std::string const& name,
        long duration,
        long count,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a pre-computed aggregated metrics event.
    /// </summary>
    /// <param name="metricData">Detailed information about the aggregated metric</param>
    /// <param name="properties">Properties of this aggregated metric event</param>
    virtual void LogAggregatedMetric(AggregatedMetricData const& metricData,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a trace event to help diagnose problems.
    /// </summary>
    /// <param name="level">Level of the trace</param>
    /// <param name="message">Detailed message of the trace</param>
    /// <param name="properties">Properties of the trace event</param>
    virtual void LogTrace(TraceLevel level,
        std::string const& message,
        EventProperties const& properties) = 0;

    /// <summary>
    /// Logs a user's state.
    /// </summary>
    /// <param name="state">User's state to be reported</param>
    /// <param name="timeToLiveInMillis">Duration in milliseconds for which the state being reported is valid for</param>
    /// <param name="properties">Properties of the user state event</param>
    virtual void LogUserState(UserState state,
        long timeToLiveInMillis,
        EventProperties const& properties) = 0;
};


}}} // namespace Microsoft::Applications::Telemetry

#endif //ARIA_ILOGGER_H
#pragma once
#include "ILogger.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
namespace Models {
// *INDENT-ON*

class ARIASDK_LIBABI AggregatedMetric
{
  public:
    /// <summary>
    /// Constructor to create an aggregated metric instance for logging auto-aggregated metric
    /// </summary>
    /// <param name="name">Name of the auto-aggregated metric</param>
    /// <param name="units">String representation of the units of the auto-aggregated metric</param>
    /// <param name="intervalInSec">Cadence in seconds to aggregate the metric</param>
    /// <param name="instanceName">Name of this metric instance like for performance counter</param>
    /// <param name="eventProperties">Properties of the auto-aggregated metric event</param>
    /// <param name="pLogger">ILogger interface pointer to use to log this aggregated metric</param>
    AggregatedMetric(std::string const& name,
        std::string const& units,
        unsigned const intervalInSec,
        EventProperties const& eventProperties,
        ILogger* pLogger = NULL);

    /// <summary>
    /// Constructor to create an aggregated metric instance for logging auto-aggregated metric
    /// </summary>
    /// <param name="name">Name of the auto-aggregated metric</param>
    /// <param name="units">String representation of the units of the auto-aggregated metric</param>
    /// <param name="intervalInSec">Cadence in seconds to aggregate the metric</param>
    /// <param name="instanceName">Name of this metric instance like for performance counter</param>
    /// <param name="objectClass">Object class for which this metric is tracking</param>
    /// <param name="objectId">Object ID for which this metric is tracking</param>
    /// <param name="eventProperties">Properties of the auto-aggregated metric event</param>
    /// <param name="pLogger">ILogger interface pointer to use to log this aggregated metric</param>
    AggregatedMetric(std::string const& name,
        std::string const& units,
        unsigned const intervalInSec,
        std::string const& instanceName,
        std::string const& objectClass,
        std::string const& objectId,
        EventProperties const& eventProperties,
        ILogger* pLogger = NULL);

    /// <summary>
    /// Destructor of the aggregated metric
    /// </summary>
    ~AggregatedMetric();

    /// <summary>
    /// Pushes a single metric value for auto-aggregation
    /// </summary>
    /// <param name="value">metric value</param>
    void PushMetric(double value);

  private:
    /// <summary>
    /// Actual impl of AggregatedMetric
    /// </summary>
    void* m_pAggregatedMetricImpl;
};

} // Models
}}}

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef AGGREGATEDMETRIC_HPP
#define AGGREGATEDMETRIC_HPP

#include "Version.hpp"

#include "ILogger.hpp"

namespace MAT_NS_BEGIN
{

    namespace Models {

        /// <summary>
        /// The AggregatedMetric class represents an aggregated metric event.
        /// </summary>
        class MATSDK_LIBABI AggregatedMetric
        {
        public:
            /// <summary>
            /// An AggregatedMetric constructor. Creates an aggregated metric instance for logging auto-aggregated metrics.
            /// </summary>
            /// <param name="name">A string that contains the name of the auto-aggregated metric.</param>
            /// <param name="units">A string that contains the units of the auto-aggregated metric.</param>
            /// <param name="intervalInSec">The polling cadence (in seconds) used to aggregate the metric.</param>
            /// <param name="eventProperties">The properties of the auto-aggregated metric event, as an EventProperties object.</param>
            /// <param name="pLogger">An ILogger interface pointer used to log this aggregated metric.</param>
            AggregatedMetric(std::string const& name,
                std::string const& units,
                unsigned const intervalInSec,
                EventProperties const& eventProperties,
                ILogger* pLogger = NULL);

            /// <summary>
            /// AggregatedMetric AggregatedMetric constructor that also takes an instance name, an object class, and an object ID. 
            /// Creates an aggregated metric instance for logging auto-aggregated metrics.
            /// </summary>
            /// <param name="name">A string that contains the name of the auto-aggregated metric.</param>
            /// <param name="units">A string that contains the units of the auto-aggregated metric.</param>
            /// <param name="intervalInSec">The polling cadence (in seconds) used to aggregate the metric.</param>
            /// <param name="instanceName">A string that contains the name of this metric instance - like for performance counter.</param>
            /// <param name="objectClass">A string that contains the object class for which this metric is trackings.</param>
            /// <param name="objectId">A string that contains the object ID for which this metric is trackings.</param>
            /// <param name="eventProperties">The properties of the auto-aggregated metric event, as an EventProperties object.</param>
            /// <param name="pLogger">An ILogger interface pointer used to log this aggregated metric.</param>
            AggregatedMetric(std::string const& name,
                std::string const& units,
                unsigned const intervalInSec,
                std::string const& instanceName,
                std::string const& objectClass,
                std::string const& objectId,
                EventProperties const& eventProperties,
                ILogger* pLogger = NULL);

            /// <summary>
            /// The AggregatedMetric destructor.
            /// </summary>
            ~AggregatedMetric();

            /// <summary>
            /// Pushes a single metric value for auto-aggregation.
            /// </summary>
            /// <param name="value">The metric value to push.</param>
            void PushMetric(double value);

        private:
            /// <summary>
            /// Actual implementation of AggregatedMetric.
            /// </summary>
            void* m_pAggregatedMetricImpl;
        };

    } // Models

} MAT_NS_END
#endif
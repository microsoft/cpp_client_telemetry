//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef SEMANTICAPIDECORATORS_HPP
#define SEMANTICAPIDECORATORS_HPP

#include "BaseDecorator.hpp"
#include "ILogger.hpp"
#include "utils/StringUtils.hpp"

namespace MAT_NS_BEGIN {

#define RECORD_EXT      record.data[0].properties

    class SemanticApiDecorators : public BaseDecorator {

    public:
        SemanticApiDecorators(ILogManager& owner) : BaseDecorator(owner) {};

        bool decorateAggregatedMetricMessage(::CsProtocol::Record& record, AggregatedMetricData const& metricData)
        {
            if (!checkNotEmpty(metricData.name, "name")) {
                return false;
            }

            record.baseType = "AggregatedMetric";
            setIfNotEmpty(RECORD_EXT, "AggregatedMetric.ObjectClass", metricData.objectClass);
            setIfNotEmpty(RECORD_EXT, "AggregatedMetric.ObjectId", metricData.objectId);
            setIfNotEmpty(RECORD_EXT, "AggregatedMetric.Name", metricData.name);
            setIfNotEmpty(RECORD_EXT, "AggregatedMetric.InstanceName", metricData.instanceName);
            setInt64Value(RECORD_EXT, "AggregatedMetric.Duration", metricData.duration);
            setInt64Value(RECORD_EXT, "AggregatedMetric.Count", metricData.count);
            setIfNotEmpty(RECORD_EXT, "AggregatedMetric.Units", metricData.units);

            for (auto const& aggr : metricData.aggregates) {
                switch (aggr.first) {
                case AggregateType_Sum:
                    setDoubleValue(RECORD_EXT, "AggregatedMetric.Aggregates.Sum", aggr.second);
                    break;

                case AggregateType_Maximum:
                    setDoubleValue(RECORD_EXT, "AggregatedMetric.Aggregates.Maximum", aggr.second);
                    break;

                case AggregateType_Minimum:
                    setDoubleValue(RECORD_EXT, "AggregatedMetric.Aggregates.Minimum", aggr.second);
                    break;

                case AggregateType_SumOfSquares:
                    setDoubleValue(RECORD_EXT, "AggregatedMetric.Aggregates.SumOfSquares", aggr.second);
                    break;

                default:
                    assert(!"unknown enum value");
                }
            }

            for (auto const& bucket : metricData.buckets) {
                setInt64Value(RECORD_EXT, "AggregatedMetric.Buckets." + toString(bucket.first), bucket.second);
            }

            return true;
        }

        bool decorateAppLifecycleMessage(::CsProtocol::Record& record, MAT::AppLifecycleState state)
        {
            static EnumValueName const names_AppLifecycleState[] = {
                { "Unknown",    0 },
            { "Launch",     1 },
            { "Exit",       2 },
            { "Suspend",    3 },
            { "Resume",     4 },
            { "Foreground", 5 },
            { "Background", 6 },
            };

            record.baseType = "AppLifecycle";
            setEnumValue(RECORD_EXT, "AppLifeCycle.State", state, names_AppLifecycleState);

            return true;
        }

        bool decorateFailureMessage(::CsProtocol::Record& record, std::string const& signature, std::string const& detail,
            std::string const& category, std::string const& id)
        {
            if (!checkNotEmpty(signature, "signature") || !checkNotEmpty(detail, "detail")) {
                return false;
            }

            record.baseType = "Failure";
            setIfNotEmpty(RECORD_EXT, "Failure.Signature", signature);
            setIfNotEmpty(RECORD_EXT, "Failure.Detail", detail);
            setIfNotEmpty(RECORD_EXT, "Failure.Category", category);
            setIfNotEmpty(RECORD_EXT, "Failure.Id", id);

            return true;
        }

        bool decoratePageActionMessage(::CsProtocol::Record& record, MAT::PageActionData const& pageActionData)
        {
            if (!checkNotEmpty(pageActionData.pageViewId, "pageViewId")) {
                return false;
            }

            static EnumValueName const names_ActionType[] = {
                { "Unspecified",          0 },
            { "Unknown",              1 },
            { "Other",                2 },
            { "Click",                3 },
            { "Pan",                  5 },
            { "Zoom",                 6 },
            { "Hover",                7 }
            };

            static EnumValueName const names_RawActionType[] = {
                { "Unspecified",          0 },
            { "Unknown",              1 },
            { "Other",                2 },
            { "LButtonDoubleClick",  11 },
            { "LButtonDown",         12 },
            { "LButtonUp",           13 },
            { "MButtonDoubleClick",  14 },
            { "MButtonDown",         15 },
            { "MButtonUp",           16 },
            { "MouseHover",          17 },
            { "MouseWheel",          18 },
            { "MouseMove",           20 },
            { "RButtonDoubleClick",  22 },
            { "RButtonDown",         23 },
            { "RButtonUp",           24 },
            { "TouchTap",            50 },
            { "TouchDoubleTap",      51 },
            { "TouchLongPress",      52 },
            { "TouchScroll",         53 },
            { "TouchPan",            54 },
            { "TouchFlick",          55 },
            { "TouchPinch",          56 },
            { "TouchZoom",           57 },
            { "TouchRotate",         58 },
            { "KeyboardPress",      100 },
            { "KeyboardEnter",      101 }
            };

            static EnumValueName const names_InputDeviceType[] = {
                { "Unspecified",          0 },
            { "Unknown",              1 },
            { "Other",                2 },
            { "Mouse",                3 },
            { "Keyboard",             4 },
            { "Touch",                5 },
            { "Stylus",               6 },
            { "Microphone",           7 },
            { "Kinect",               8 },
            { "Camera",               9 }
            };

            record.baseType = "PageAction";
            setEnumValue(RECORD_EXT, "PageAction.ActionType", pageActionData.actionType, names_ActionType);
            setIfNotEmpty(RECORD_EXT, "PageAction.PageViewId", pageActionData.pageViewId);
            setEnumValue(RECORD_EXT, "PageAction.RawActionType", pageActionData.rawActionType, names_RawActionType);
            setEnumValue(RECORD_EXT, "PageAction.InputDeviceType", pageActionData.inputDeviceType, names_InputDeviceType);
            setIfNotEmpty(RECORD_EXT, "PageAction.DestinationUri", pageActionData.destinationUri);
            setIfNotEmpty(RECORD_EXT, "PageAction.TargetItemId", pageActionData.targetItemId);
            setIfNotEmpty(RECORD_EXT, "PageAction.TargetItemDataSource.Name", pageActionData.targetItemDataSourceName);
            setIfNotEmpty(RECORD_EXT, "PageAction.TargetItemDataSource.Category", pageActionData.targetItemDataSourceCategory);
            setIfNotEmpty(RECORD_EXT, "PageAction.TargetItemDataSource.Collection", pageActionData.targetItemDataSourceCollection);
            setIfNotEmpty(RECORD_EXT, "PageAction.TargetItemLayout.Container", pageActionData.targetItemLayoutContainer);
            setInt64Value(RECORD_EXT, "PageAction.TargetItemLayout.Rank", pageActionData.targetItemLayoutRank);

            return true;
        }

        bool decoratePageViewMessage(::CsProtocol::Record& record, std::string const& id, std::string const& pageName,
            std::string const& category, std::string const& uri, std::string const& referrer)
        {
            if (!checkNotEmpty(id, "id")) {
                return false;
            }

            record.baseType = "PageView";
            setIfNotEmpty(RECORD_EXT, "PageView.Id", id);
            setIfNotEmpty(RECORD_EXT, "PageView.Name", pageName);
            setIfNotEmpty(RECORD_EXT, "PageView.Category", category);
            setIfNotEmpty(RECORD_EXT, "PageView.Uri", uri);
            setIfNotEmpty(RECORD_EXT, "PageView.ReferrerUri", referrer);

            return true;
        }

        bool decorateSampledMetricMessage(::CsProtocol::Record& record, std::string const& name, double value, std::string const& units,
            std::string const& instanceName, std::string const& objectClass, std::string const& objectId)
        {
            if (!checkNotEmpty(name, "name") || !checkNotEmpty(units, "units")) {
                return false;
            }

            record.baseType = "SampledMetric";
            setIfNotEmpty(RECORD_EXT, "SampledMetric.Name", name);
            setDoubleValue(RECORD_EXT, "SampledMetric.Value", value);
            setIfNotEmpty(RECORD_EXT, "SampledMetric.Units", units);
            setIfNotEmpty(RECORD_EXT, "SampledMetric.InstanceName", instanceName);
            setIfNotEmpty(RECORD_EXT, "SampledMetric.ObjectClass", objectClass);
            setIfNotEmpty(RECORD_EXT, "SampledMetric.ObjectId", objectId);

            return true;
        }

        bool decorateTraceMessage(::CsProtocol::Record& record, TraceLevel const& level, std::string const& message)
        {
            if (!checkNotEmpty(message, "message")) {
                return false;
            }

            static EnumValueName const names_TraceLevel[] = {
                { "None",        0 },
            { "Error",       1 },
            { "Warning",     2 },
            { "Information", 3 },
            { "Verbose",     4 }
            };

            record.baseType = "Trace";
            setEnumValue(RECORD_EXT, "Trace.Level", level, names_TraceLevel);
            setIfNotEmpty(RECORD_EXT, "Trace.Message", message);

            return true;
        }

        bool decorateUserStateMessage(::CsProtocol::Record& record, UserState state, long expiry)
        {
            static EnumValueName const names_UserState[] = {
                { "Unknown",   0 },
            { "Connected", 1 },
            { "Reachable", 2 },
            { "SignedIn",  3 },
            { "SignedOut", 4 }
            };

            record.baseType = "UserInfo_UserState";
            setIfNotEmpty(RECORD_EXT, "State.Name", "UserState");
            setEnumValue(RECORD_EXT, "State.Value", state, names_UserState);
            setInt64Value(RECORD_EXT, "State.TimeToLive", expiry);
            setBoolValue(RECORD_EXT, "State.IsTransition", true);

            return true;
        }

        static std::string SessionDurationBucket(const int64_t time_in_seconds)
        {
            if (time_in_seconds < 0)
                return "Undefined";
            if (time_in_seconds <= 3)
                return "UpTo3Sec";
            if (time_in_seconds <= 10)
                return "UpTo10Sec";
            if (time_in_seconds <= 30)
                return "UpTo30Sec";
            if (time_in_seconds <= 60)
                return "UpTo60Sec";
            if (time_in_seconds <= 180)
                return "UpTo3Min";
            if (time_in_seconds <= 600)
                return "UpTo10Min";
            if (time_in_seconds <= 1800)
                return "UpTo30Min";

            return "Above30Min";
        }

        bool decorateSessionMessage(::CsProtocol::Record& record,
            SessionState state,
            std::string const& id,
            std::string const& firstTime,
            std::string const& sdkuid,
            int64_t duration)
        {
            std::string stateString = (state == SessionState::Session_Started) ? "Started" : "Ended";

            record.baseType = "Session";
            setIfNotEmpty(RECORD_EXT, SESSION_STATE, stateString);
            setIfNotEmpty(RECORD_EXT, SESSION_ID, id);
            setIfNotEmpty(RECORD_EXT, SESSION_FIRST_TIME, firstTime);
            setIfNotEmpty(RECORD_EXT, SESSION_SDKUID, sdkuid);

            if (duration > 0)
            {   // This fields are added only for the ended session
                setInt64Value(RECORD_EXT, SESSION_DURATION, duration);
                setIfNotEmpty(RECORD_EXT, SESSION_DURATION_BUCKET, SemanticApiDecorators::SessionDurationBucket(duration));
            }

            return true;
        }

    };

} MAT_NS_END
#endif


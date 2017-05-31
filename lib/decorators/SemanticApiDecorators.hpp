// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "aria\Config.hpp"
#include <aria\ILogger.hpp>

namespace ARIASDK_NS_BEGIN {


class ARIASDK_LIBABI SemanticApiDecorators : public DecoratorBase {
  public:
    SemanticApiDecorators()
    {
    }

    bool decorateAggregatedMetricMessage(::AriaProtocol::Record& record, AggregatedMetricData const& metricData)
    {
        if (!checkNotEmpty(metricData.name, "name")) {
            return false;
        }

        record.EventType = "AggregatedMetric";
        setIfNotEmpty(record.Extension,           "AggregatedMetric.ObjectClass",  metricData.objectClass);
        setIfNotEmpty(record.Extension,           "AggregatedMetric.ObjectId",     metricData.objectId);
        setIfNotEmpty(record.Extension,           "AggregatedMetric.Name",         metricData.name);
        setIfNotEmpty(record.Extension,           "AggregatedMetric.InstanceName", metricData.instanceName);
        setInt64Value(record.TypedExtensionInt64, "AggregatedMetric.Duration",     metricData.duration);
        setInt64Value(record.TypedExtensionInt64, "AggregatedMetric.Count",        metricData.count);
        setIfNotEmpty(record.Extension,           "AggregatedMetric.Units",        metricData.units);

        for (auto const& aggr : metricData.aggregates) {
            switch (aggr.first) {
                case AggregateType_Sum:
                    setDoubleValue(record.TypedExtensionDouble, "AggregatedMetric.Aggregates.Sum", aggr.second);
                    break;

                case AggregateType_Maximum:
                    setDoubleValue(record.TypedExtensionDouble, "AggregatedMetric.Aggregates.Maximum", aggr.second);
                    break;

                case AggregateType_Minimum:
                    setDoubleValue(record.TypedExtensionDouble, "AggregatedMetric.Aggregates.Minimum", aggr.second);
                    break;

                case AggregateType_SumOfSquares:
                    setDoubleValue(record.TypedExtensionDouble, "AggregatedMetric.Aggregates.SumOfSquares", aggr.second);
                    break;

                default:
                    assert(!"unknown enum value");
            }
        }

        for (auto const& bucket : metricData.buckets) {
            setInt64Value(record.TypedExtensionInt64, "AggregatedMetric.Buckets." + toString(bucket.first), bucket.second);
        }

        return true;
    }

    bool decorateAppLifecycleMessage(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::AppLifecycleState state)
    {
        static EnumValueName const names_AppLifecycleState[] = {
            {"Unknown",    0},
            {"Launch",     1},
            {"Exit",       2},
            {"Suspend",    3},
            {"Resume",     4},
            {"Foreground", 5},
            {"Background", 6},
        };

        record.EventType = "AppLifecycle";
        setEnumValue(record.Extension, "AppLifeCycle.State", state, names_AppLifecycleState);

        return true;
    }

    bool decorateFailureMessage(::AriaProtocol::Record& record, std::string const& signature, std::string const& detail,
        std::string const& category, std::string const& id)
    {
        if (!checkNotEmpty(signature, "signature") || !checkNotEmpty(detail, "detail")) {
            return false;
        }

        record.EventType = "Failure";
        setIfNotEmpty(record.Extension, "Failure.Signature", signature);
        setIfNotEmpty(record.Extension, "Failure.Detail",    detail);
        setIfNotEmpty(record.Extension, "Failure.Category",  category);
        setIfNotEmpty(record.Extension, "Failure.Id",        id);

        return true;
    }

    bool decoratePageActionMessage(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::PageActionData const& pageActionData)
    {
        if (!checkNotEmpty(pageActionData.pageViewId, "pageViewId")) {
            return false;
        }

        static EnumValueName const names_ActionType[] = {
            {"Unspecified",          0},
            {"Unknown",              1},
            {"Other",                2},
            {"Click",                3},
            {"Pan",                  5},
            {"Zoom",                 6},
            {"Hover",                7}
        };

        static EnumValueName const names_RawActionType[] = {
            {"Unspecified",          0},
            {"Unknown",              1},
            {"Other",                2},
            {"LButtonDoubleClick",  11},
            {"LButtonDown",         12},
            {"LButtonUp",           13},
            {"MButtonDoubleClick",  14},
            {"MButtonDown",         15},
            {"MButtonUp",           16},
            {"MouseHover",          17},
            {"MouseWheel",          18},
            {"MouseMove",           20},
            {"RButtonDoubleClick",  22},
            {"RButtonDown",         23},
            {"RButtonUp",           24},
            {"TouchTap",            50},
            {"TouchDoubleTap",      51},
            {"TouchLongPress",      52},
            {"TouchScroll",         53},
            {"TouchPan",            54},
            {"TouchFlick",          55},
            {"TouchPinch",          56},
            {"TouchZoom",           57},
            {"TouchRotate",         58},
            {"KeyboardPress",      100},
            {"KeyboardEnter",      101}
        };

        static EnumValueName const names_InputDeviceType[] = {
            {"Unspecified",          0},
            {"Unknown",              1},
            {"Other",                2},
            {"Mouse",                3},
            {"Keyboard",             4},
            {"Touch",                5},
            {"Stylus",               6},
            {"Microphone",           7},
            {"Kinect",               8},
            {"Camera",               9}
        };

        record.EventType = "PageAction";
        setEnumValue(record.Extension,            "PageAction.ActionType",                      pageActionData.actionType,      names_ActionType);
        setIfNotEmpty(record.Extension,           "PageAction.PageViewId",                      pageActionData.pageViewId);
        setEnumValue(record.Extension,            "PageAction.RawActionType",                   pageActionData.rawActionType,   names_RawActionType);
        setEnumValue(record.Extension,            "PageAction.InputDeviceType",                 pageActionData.inputDeviceType, names_InputDeviceType);
        setIfNotEmpty(record.Extension,           "PageAction.DestinationUri",                  pageActionData.destinationUri);
        setIfNotEmpty(record.Extension,           "PageAction.TargetItemId",                    pageActionData.targetItemId);
        setIfNotEmpty(record.Extension,           "PageAction.TargetItemDataSource.Name",       pageActionData.targetItemDataSourceName);
        setIfNotEmpty(record.Extension,           "PageAction.TargetItemDataSource.Category",   pageActionData.targetItemDataSourceCategory);
        setIfNotEmpty(record.Extension,           "PageAction.TargetItemDataSource.Collection", pageActionData.targetItemDataSourceCollection);
        setIfNotEmpty(record.Extension,           "PageAction.TargetItemLayout.Container",      pageActionData.targetItemLayoutContainer);
        setInt64Value(record.TypedExtensionInt64, "PageAction.TargetItemLayout.Rank",           pageActionData.targetItemLayoutRank);

        return true;
    }

    bool decoratePageViewMessage(::AriaProtocol::Record& record, std::string const& id, std::string const& pageName,
        std::string const& category, std::string const& uri, std::string const& referrer)
    {
        if (!checkNotEmpty(id, "id")) {
            return false;
        }

        record.EventType = "PageView";
        setIfNotEmpty(record.Extension, "PageView.Id",          id);
        setIfNotEmpty(record.Extension, "PageView.Name",        pageName);
        setIfNotEmpty(record.Extension, "PageView.Category",    category);
        setIfNotEmpty(record.Extension, "PageView.Uri",         uri);
        setIfNotEmpty(record.Extension, "PageView.ReferrerUri", referrer);

        return true;
    }

    bool decorateSampledMetricMessage(::AriaProtocol::Record& record, std::string const& name, double value, std::string const& units,
        std::string const& instanceName, std::string const& objectClass, std::string const& objectId)
    {
        if (!checkNotEmpty(name, "name") || !checkNotEmpty(units, "units")) {
            return false;
        }

        record.EventType = "SampledMetric";
        setIfNotEmpty(record.Extension,            "SampledMetric.Name",         name);
        setDoubleValue(record.TypedExtensionDouble, "SampledMetric.Value", value);
        setIfNotEmpty(record.Extension,            "SampledMetric.Units",        units);
        setIfNotEmpty(record.Extension,            "SampledMetric.InstanceName", instanceName);
        setIfNotEmpty(record.Extension,            "SampledMetric.ObjectClass",  objectClass);
        setIfNotEmpty(record.Extension,            "SampledMetric.ObjectId",     objectId);

        return true;
    }

    bool decorateTraceMessage(::AriaProtocol::Record& record, TraceLevel const& level, std::string const& message)
    {
        if (!checkNotEmpty(message, "message")) {
            return false;
        }

        static EnumValueName const names_TraceLevel[] = {
            {"None",        0},
            {"Error",       1},
            {"Warning",     2},
            {"Information", 3},
            {"Verbose",     4}
        };

        record.EventType = "Trace";
        setEnumValue(record.Extension,  "Trace.Level",   level, names_TraceLevel);
        setIfNotEmpty(record.Extension, "Trace.Message", message);

        return true;
    }

    bool decorateUserStateMessage(::AriaProtocol::Record& record, UserState state, long expiry)
    {
        static EnumValueName const names_UserState[] = {
            {"Unknown",   0},
            {"Connected", 1},
            {"Reachable", 2},
            {"SignedIn",  3},
            {"SignedOut", 4}
        };

        record.EventType = "UserInfo_UserState";
        setIfNotEmpty(record.Extension,             "State.Name",         "UserState");
        setEnumValue(record.Extension,              "State.Value",        state, names_UserState);
        setInt64Value(record.TypedExtensionInt64,   "State.TimeToLive",   expiry);
        setBoolValue(record.TypedExtensionBoolean, "State.IsTransition", true);

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


    bool decorateSessionMessage(::AriaProtocol::Record& record,
                                SessionState state, 
                                std::string const& id,
                                std::string const& firstTime,
                                std::string const& sdkuid,
                                int64_t duration)
    {
        std::string stateString = (state == SessionState::Session_Started) ? "Started" : "Ended";

        record.EventType = "Session";
        setIfNotEmpty(record.Extension, SESSION_STATE, stateString);
        setIfNotEmpty(record.Extension, SESSION_ID, id);
        setIfNotEmpty(record.Extension, SESSION_FIRST_TIME, firstTime);
        setIfNotEmpty(record.Extension, SESSION_SDKUID, sdkuid);

        if (duration >  0)
        {   // This fields are added only for the ended session
            setInt64Value(record.TypedExtensionInt64, SESSION_DURATION, duration);
            setIfNotEmpty(record.Extension,           SESSION_DURATION_BUCKET, SemanticApiDecorators::SessionDurationBucket(duration));
        }
                    
        return true;
    }
};
} ARIASDK_NS_END

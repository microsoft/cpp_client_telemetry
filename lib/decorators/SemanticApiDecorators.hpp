// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "Config.hpp"
#include <ILogger.hpp>

namespace ARIASDK_NS_BEGIN {


class SemanticApiDecorators : public DecoratorBase {
  public:
    SemanticApiDecorators()
    {
    }

    bool decorateAggregatedMetricMessage(::AriaProtocol::CsEvent& record, AggregatedMetricData const& metricData)
    {
        if (!checkNotEmpty(metricData.name, "name")) {
            return false;
        }
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }

        record.baseType = "AggregatedMetric";
        setIfNotEmpty(record.data[0].properties,           "AggregatedMetric.ObjectClass",  metricData.objectClass);
        setIfNotEmpty(record.data[0].properties,           "AggregatedMetric.ObjectId",     metricData.objectId);
        setIfNotEmpty(record.data[0].properties,           "AggregatedMetric.Name",         metricData.name);
        setIfNotEmpty(record.data[0].properties,           "AggregatedMetric.InstanceName", metricData.instanceName);
        setInt64Value(record.data[0].properties,           "AggregatedMetric.Duration",     metricData.duration);
        setInt64Value(record.data[0].properties,           "AggregatedMetric.Count",        metricData.count);
        setIfNotEmpty(record.data[0].properties,           "AggregatedMetric.Units",        metricData.units);

        for (auto const& aggr : metricData.aggregates) {
            switch (aggr.first) {
                case AggregateType_Sum:
                    setDoubleValue(record.data[0].properties, "AggregatedMetric.Aggregates.Sum", aggr.second);
                    break;

                case AggregateType_Maximum:
                    setDoubleValue(record.data[0].properties, "AggregatedMetric.Aggregates.Maximum", aggr.second);
                    break;

                case AggregateType_Minimum:
                    setDoubleValue(record.data[0].properties, "AggregatedMetric.Aggregates.Minimum", aggr.second);
                    break;

                case AggregateType_SumOfSquares:
                    setDoubleValue(record.data[0].properties, "AggregatedMetric.Aggregates.SumOfSquares", aggr.second);
                    break;

                default:
                    assert(!"unknown enum value");
            }
        }

        for (auto const& bucket : metricData.buckets) {
            setInt64Value(record.data[0].properties, "AggregatedMetric.Buckets." + toString(bucket.first), bucket.second);
        }

        return true;
    }

    bool decorateAppLifecycleMessage(::AriaProtocol::CsEvent& record, ::Microsoft::Applications::Telemetry::AppLifecycleState state)
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

        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        record.baseType = "AppLifecycle";
        setEnumValue(record.data[0].properties, "AppLifeCycle.State", state, names_AppLifecycleState);

        return true;
    }

    bool decorateFailureMessage(::AriaProtocol::CsEvent& record, std::string const& signature, std::string const& detail,
        std::string const& category, std::string const& id)
    {
        if (!checkNotEmpty(signature, "signature") || !checkNotEmpty(detail, "detail")) {
            return false;
        }

        record.baseType = "Failure";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setIfNotEmpty(record.data[0].properties, "Failure.Signature", signature);
        setIfNotEmpty(record.data[0].properties, "Failure.Detail",    detail);
        setIfNotEmpty(record.data[0].properties, "Failure.Category",  category);
        setIfNotEmpty(record.data[0].properties, "Failure.Id",        id);

        return true;
    }

    bool decoratePageActionMessage(::AriaProtocol::CsEvent& record, ::Microsoft::Applications::Telemetry::PageActionData const& pageActionData)
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

        record.baseType = "PageAction";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setEnumValue(record.data[0].properties,            "PageAction.ActionType",                      pageActionData.actionType,      names_ActionType);
        setIfNotEmpty(record.data[0].properties,           "PageAction.PageViewId",                      pageActionData.pageViewId);
        setEnumValue(record.data[0].properties,            "PageAction.RawActionType",                   pageActionData.rawActionType,   names_RawActionType);
        setEnumValue(record.data[0].properties,            "PageAction.InputDeviceType",                 pageActionData.inputDeviceType, names_InputDeviceType);
        setIfNotEmpty(record.data[0].properties,           "PageAction.DestinationUri",                  pageActionData.destinationUri);
        setIfNotEmpty(record.data[0].properties,           "PageAction.TargetItemId",                    pageActionData.targetItemId);
        setIfNotEmpty(record.data[0].properties,           "PageAction.TargetItemDataSource.Name",       pageActionData.targetItemDataSourceName);
        setIfNotEmpty(record.data[0].properties,           "PageAction.TargetItemDataSource.Category",   pageActionData.targetItemDataSourceCategory);
        setIfNotEmpty(record.data[0].properties,           "PageAction.TargetItemDataSource.Collection", pageActionData.targetItemDataSourceCollection);
        setIfNotEmpty(record.data[0].properties,           "PageAction.TargetItemLayout.Container",      pageActionData.targetItemLayoutContainer);
        setInt64Value(record.data[0].properties,           "PageAction.TargetItemLayout.Rank",           pageActionData.targetItemLayoutRank);

        return true;
    }

    bool decoratePageViewMessage(::AriaProtocol::CsEvent& record, std::string const& id, std::string const& pageName,
        std::string const& category, std::string const& uri, std::string const& referrer)
    {
        if (!checkNotEmpty(id, "id")) {
            return false;
        }

        record.baseType = "PageView";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setIfNotEmpty(record.data[0].properties, "PageView.Id",          id);
        setIfNotEmpty(record.data[0].properties, "PageView.Name",        pageName);
        setIfNotEmpty(record.data[0].properties, "PageView.Category",    category);
        setIfNotEmpty(record.data[0].properties, "PageView.Uri",         uri);
        setIfNotEmpty(record.data[0].properties, "PageView.ReferrerUri", referrer);

        return true;
    }

    bool decorateSampledMetricMessage(::AriaProtocol::CsEvent& record, std::string const& name, double value, std::string const& units,
        std::string const& instanceName, std::string const& objectClass, std::string const& objectId)
    {
        if (!checkNotEmpty(name, "name") || !checkNotEmpty(units, "units")) {
            return false;
        }

        record.baseType = "SampledMetric";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setIfNotEmpty(record.data[0].properties,            "SampledMetric.Name",         name);
        setDoubleValue(record.data[0].properties,           "SampledMetric.Value",        value);
        setIfNotEmpty(record.data[0].properties,            "SampledMetric.Units",        units);
        setIfNotEmpty(record.data[0].properties,            "SampledMetric.InstanceName", instanceName);
        setIfNotEmpty(record.data[0].properties,            "SampledMetric.ObjectClass",  objectClass);
        setIfNotEmpty(record.data[0].properties,            "SampledMetric.ObjectId",     objectId);

        return true;
    }

    bool decorateTraceMessage(::AriaProtocol::CsEvent& record, TraceLevel const& level, std::string const& message)
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

        record.baseType = "Trace";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setEnumValue(record.data[0].properties,  "Trace.Level",   level, names_TraceLevel);
        setIfNotEmpty(record.data[0].properties, "Trace.Message", message);

        return true;
    }

    bool decorateUserStateMessage(::AriaProtocol::CsEvent& record, UserState state, long expiry)
    {
        static EnumValueName const names_UserState[] = {
            {"Unknown",   0},
            {"Connected", 1},
            {"Reachable", 2},
            {"SignedIn",  3},
            {"SignedOut", 4}
        };

        record.baseType = "UserInfo_UserState";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setIfNotEmpty(record.data[0].properties,             "State.Name",         "UserState");
        setEnumValue(record.data[0].properties,              "State.Value",        state, names_UserState);
        setInt64Value(record.data[0].properties,             "State.TimeToLive",   expiry);
        setBoolValue(record.data[0].properties,              "State.IsTransition", true);

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


    bool decorateSessionMessage(::AriaProtocol::CsEvent& record,
                                SessionState state, 
                                std::string const& id,
                                std::string const& firstTime,
                                std::string const& sdkuid,
                                int64_t duration)
    {
        std::string stateString = (state == SessionState::Session_Started) ? "Started" : "Ended";

        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        record.baseType = "Session";
        if (record.data.size() == 0)
        {
            ::AriaProtocol::Data data;
            record.data.push_back(data);
        }
        setIfNotEmpty(record.data[0].properties, SESSION_STATE, stateString);
        setIfNotEmpty(record.data[0].properties, SESSION_ID, id);
        setIfNotEmpty(record.data[0].properties, SESSION_FIRST_TIME, firstTime);
        setIfNotEmpty(record.data[0].properties, SESSION_SDKUID, sdkuid);

        if (duration >  0)
        {   // This fields are added only for the ended session
            setInt64Value(record.data[0].properties, SESSION_DURATION, duration);
            setIfNotEmpty(record.data[0].properties, SESSION_DURATION_BUCKET, SemanticApiDecorators::SessionDurationBucket(duration));
        }
                    
        return true;
    }
};
} ARIASDK_NS_END

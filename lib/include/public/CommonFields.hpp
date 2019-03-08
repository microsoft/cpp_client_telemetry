// Copyright (c) Microsoft. All rights reserved.
#ifndef MAT_COMMONFIELDS_HPP
#define MAT_COMMONFIELDS_HPP

#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{
    /* Aria-v1 legacy protocol common property names supported by 1DS SDK */
    const char* const EVENTRECORD_TYPE_CUSTOM_EVENT                   = "custom";

    const char* const COMMONFIELDS_APP_ID                             = "AppInfo.Id";
    const char* const COMMONFIELDS_APP_VERSION                        = "AppInfo.Version";
    const char* const COMMONFIELDS_APP_LANGUAGE                       = "AppInfo.Language";
    const char* const COMMONFIELDS_APP_EXPERIMENTIDS                  = "AppInfo.ExperimentIds";
    const char* const COMMONFIELDS_APP_EXPERIMENTETAG                 = "AppInfo.ETag";
    const char* const COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID       = "AppInfo.ImpressionId";

    const char* const COMMONFIELDS_DEVICE_ID                          = "DeviceInfo.Id";
    const char* const COMMONFIELDS_DEVICE_MAKE                        = "DeviceInfo.Make";
    const char* const COMMONFIELDS_DEVICE_MODEL                       = "DeviceInfo.Model";
    const char* const COMMONFIELDS_DEVICE_CLASS                       = "DeviceInfo.Class";

    const char* const COMMONFIELDS_NETWORK_PROVIDER                   = "DeviceInfo.NetworkProvider";
    const char* const COMMONFIELDS_NETWORK_TYPE                       = "DeviceInfo.NetworkType";
    const char* const COMMONFIELDS_NETWORK_COST                       = "DeviceInfo.NetworkCost";

    const char* const COMMONFIELDS_OS_NAME                            = "DeviceInfo.OsName";
    const char* const COMMONFIELDS_OS_VERSION                         = "DeviceInfo.OsVersion";
    const char* const COMMONFIELDS_OS_BUILD                           = "DeviceInfo.OsBuild";

    const char* const COMMONFIELDS_COMMERCIAL_ID                      = "M365aInfo.EnrolledTenantId";

    const char* const COMMONFIELDS_USER_ID                            = "UserInfo.Id";
    const char* const COMMONFIELDS_USER_MSAID                         = "UserInfo.MsaId";
    const char* const COMMONFIELDS_USER_ANID                          = "UserInfo.ANID";
    const char* const COMMONFIELDS_USER_ADVERTISINGID                 = "UserInfo.AdvertisingId";
    const char* const COMMONFIELDS_USER_LANGUAGE                      = "UserInfo.Language";
    const char* const COMMONFIELDS_USER_TIMEZONE                      = "UserInfo.TimeZone";

    const char* const COMMONFIELDS_PIPELINEINFO_ACCOUNT               = "PipelineInfo.AccountId";

    const char* const COMMONFIELDS_EVENT_TIME                         = "EventInfo.Time";
    const char* const COMMONFIELDS_EVENT_SDKVERSION                   = "EventInfo.SdkVersion";
    const char* const COMMONFIELDS_EVENT_CRC32                        = "EventInfo.CRC32";
    const char* const COMMONFIELDS_EVENT_SOURCE                       = "EventInfo.Source";
    const char* const COMMONFIELDS_EVENT_NAME                         = "EventInfo.Name";
    const char* const COMMONFIELDS_EVENT_INITID                       = "EventInfo.InitId";
    const char* const COMMONFIELDS_EVENT_SEQ                          = "EventInfo.Sequence";

    const char* const SESSION_FIRST_TIME                              = "Session.FirstLaunchTime";
    const char* const SESSION_STATE                                   = "Session.State";
    const char* const SESSION_ID                                      = "Session.Id";
    const char* const SESSION_DURATION                                = "Session.Duration";
    const char* const SESSION_DURATION_BUCKET                         = "Session.DurationBucket";
    const char* const SESSION_SDKUID                                  = "DeviceInfo.SDKUid";

} ARIASDK_NS_END

#endif

/* Copyright (c) Microsoft. All rights reserved. */
#ifndef MAT_COMMONFIELDS_H
#define MAT_COMMONFIELDS_H

#define EVENTRECORD_TYPE_CUSTOM_EVENT                        "custom"

#define COMMONFIELDS_IKEY                                    "iKey"

#define COMMONFIELDS_APP_ENV                                 "AppInfo.Env"
#define COMMONFIELDS_APP_ID                                  "AppInfo.Id"
#define COMMONFIELDS_APP_NAME                                "AppInfo.Name"
#define COMMONFIELDS_APP_VERSION                             "AppInfo.Version"
#define COMMONFIELDS_APP_LANGUAGE                            "AppInfo.Language"

#define COMMONFIELDS_APP_EXPERIMENTIDS                       "AppInfo.ExperimentIds"
#define COMMONFIELDS_APP_EXPERIMENTETAG                      "AppInfo.ETag"
#define COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID            "AppInfo.ImpressionId"

#define COMMONFIELDS_DEVICE_ID                               "DeviceInfo.Id"
#define COMMONFIELDS_DEVICE_MAKE                             "DeviceInfo.Make"
#define COMMONFIELDS_DEVICE_MODEL                            "DeviceInfo.Model"
#define COMMONFIELDS_DEVICE_CLASS                            "DeviceInfo.Class"

#define COMMONFIELDS_NETWORK_PROVIDER                        "DeviceInfo.NetworkProvider"
#define COMMONFIELDS_NETWORK_TYPE                            "DeviceInfo.NetworkType"
#define COMMONFIELDS_NETWORK_COST                            "DeviceInfo.NetworkCost"

#define COMMONFIELDS_OS_NAME                                 "DeviceInfo.OsName"
#define COMMONFIELDS_OS_VERSION                              "DeviceInfo.OsVersion"
#define COMMONFIELDS_OS_BUILD                                "DeviceInfo.OsBuild"

#define COMMONFIELDS_COMMERCIAL_ID                           "M365aInfo.EnrolledTenantId"

#define COMMONFIELDS_USER_ID                                 "UserInfo.Id"
#define COMMONFIELDS_USER_MSAID                              "UserInfo.MsaId"
#define COMMONFIELDS_USER_ANID                               "UserInfo.ANID"
#define COMMONFIELDS_USER_ADVERTISINGID                      "UserInfo.AdvertisingId"
#define COMMONFIELDS_USER_LANGUAGE                           "UserInfo.Language"
#define COMMONFIELDS_USER_TIMEZONE                           "UserInfo.TimeZone"

#define COMMONFIELDS_PIPELINEINFO_ACCOUNT                    "PipelineInfo.AccountId"

#define COMMONFIELDS_EVENT_TIME                              "EventInfo.Time"
#define COMMONFIELDS_EVENT_SDKVERSION                        "EventInfo.SdkVersion"
#define COMMONFIELDS_EVENT_CRC32                             "EventInfo.CRC32"
#define COMMONFIELDS_EVENT_SOURCE                            "EventInfo.Source"
#define COMMONFIELDS_EVENT_NAME                              "EventInfo.Name"
#define COMMONFIELDS_EVENT_INITID                            "EventInfo.InitId"
#define COMMONFIELDS_EVENT_SEQ                               "EventInfo.Sequence"
#define COMMONFIELDS_EVENT_PRIVTAGS                          "EventInfo.PrivTags"
#define COMMONFIELDS_EVENT_PRIVLEVEL                         "EventInfo.PrivLevel"
#define COMMONFIELDS_EVENT_LEVEL                             "EventInfo.Level"
#define COMMONFIELDS_EVENT_PRIORITY                          "EventInfo.Priority"
#define COMMONFIELDS_EVENT_LATENCY                           "EventInfo.Latency"
#define COMMONFIELDS_EVENT_PERSISTENCE                       "EventInfo.Persistence"
#define COMMONFIELDS_EVENT_POLICYFLAGS                       "EventInfo.PolicyFlags"

#define SESSION_FIRST_TIME                                   "Session.FirstLaunchTime"
#define SESSION_STATE                                        "Session.State"
#define SESSION_ID                                           "Session.Id"
#define SESSION_DURATION                                     "Session.Duration"
#define SESSION_DURATION_BUCKET                              "Session.DurationBucket"
#define SESSION_ID_LEGACY                                    "act_session_id"

#define COMMONFIELDS_METADATA_VIEWINGPRODUCERID              "MetaData.ViewingProducerId"
#define COMMONFIELDS_METADATA_VIEWINGCATEGORY                "MetaData.ViewingCategory"
#define COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH      "MetaData.ViewingPayloadDecoderPath"
#define COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME "MetaData.ViewingPayloadEncodedFieldName"
#define COMMONFIELDS_METADATA_VIEWINGEXTRA1                  "MetaData.ViewingExtra1"
#define COMMONFIELDS_METADATA_VIEWINGEXTRA2                  "MetaData.ViewingExtra2"
#define COMMONFIELDS_METADATA_VIEWINGEXTRA3                  "MetaData.ViewingExtra3"

/* Scope configuration parameter defines the context fields inheritance rules */
#define CONTEXT_SCOPE_EMPTY                             ""   /* Default rule for environment        */
#define CONTEXT_SCOPE_ALL                               "*"  /* Inherit all parent context props    */
#define CONTEXT_SCOPE_NONE                              "-"  /* Do not inherit parent context props */

/* Default set of diagnostic level constants. Customers may define their own set. */
#define DIAG_LEVEL_NOTSET                               255
#define DIAG_LEVEL_DEFAULT                              DIAG_LEVEL_NOTSET     /* Default level is inherited from parent */
#define DIAG_LEVEL_NONE                                 0       /* Logging disabled                       */

/* Windows OS diagnostic level classification        */
#define DIAG_LEVEL_BASIC                                1       /* Basic info                             */
#define DIAG_LEVEL_ENHANCED                             2       /* Additional performance data            */
#define DIAG_LEVEL_FULL                                 3       /* Extra activity and enhanced reporting  */

/* Microsoft Office diagnostic level classification  */
#define DIAG_LEVEL_REQUIRED                             1       /* Data that we need to collect in order to keep the product secure, up to date, and performing as expected */
#define DIAG_LEVEL_OPTIONAL                             2       /* Additional optional data               */

/* Custom SDK configuration allows to override DIAG_LEVEL_DEFAULT_MIN and DIAG_LEVEL_DEFAULT_MAX          */
#ifndef DIAG_LEVEL_DEFAULT_MIN
#define DIAG_LEVEL_DEFAULT_MIN                          DIAG_LEVEL_REQUIRED
#endif

#ifndef DIAG_LEVEL_DEFAULT_MAX
#define DIAG_LEVEL_DEFAULT_MAX                          DIAG_LEVEL_OPTIONAL
#endif

/* TODO: [MG] - this field does not exist in Common Schema. Move it away from DeviceInfo namespace */
#define SESSION_SDKUID                                  "DeviceInfo.SDKUid"

#define SETTER_METHOD(NAME) Set ## NAME
#define DECLARE_COMMONFIELD(name, placeholder)              \
    virtual void SETTER_METHOD (name) (const std::string & x)         \
    {                                                       \
        SetCommonField(placeholder, x);                     \
    };

#endif

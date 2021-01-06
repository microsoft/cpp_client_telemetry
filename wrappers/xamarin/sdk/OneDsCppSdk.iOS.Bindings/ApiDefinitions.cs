//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using Foundation;
using ObjCRuntime;

namespace Microsoft.Applications.Events
{
    // @interface ODWCommonDataContext : NSObject
    [BaseType (typeof(NSObject), Name = "ODWCommonDataContext")]
    public interface CommonDataContext
    {
        // @property (readwrite, copy, nonatomic) NSString * _Nonnull DomainName;
        [Export ("DomainName")]
        string DomainName { get; set; }

        // @property (readwrite, copy, nonatomic) NSString * _Nonnull MachineName;
        [Export ("MachineName")]
        string MachineName { get; set; }

        // @property (readwrite, copy, nonatomic) NSString * _Nonnull UserName;
        [Export ("UserName")]
        string UserName { get; set; }

        // @property (readwrite, copy, nonatomic) NSString * _Nonnull UserAlias;
        [Export ("UserAlias")]
        string UserAlias { get; set; }

        // @property (readwrite, copy, nonatomic) NSMutableArray * _Nonnull IpAddresses;
        [Export ("IpAddresses", ArgumentSemantic.Copy)]
        NSMutableArray IpAddresses { get; set; }

        // @property (readwrite, copy, nonatomic) NSMutableArray * _Nonnull LanguageIdentifiers;
        [Export ("LanguageIdentifiers", ArgumentSemantic.Copy)]
        NSMutableArray LanguageIdentifiers { get; set; }

        // @property (readwrite, copy, nonatomic) NSMutableArray * _Nonnull MachineIds;
        [Export ("MachineIds", ArgumentSemantic.Copy)]
        NSMutableArray MachineIds { get; set; }

        // @property (readwrite, copy, nonatomic) NSMutableArray * _Nonnull OutOfScopeIdentifiers;
        [Export ("OutOfScopeIdentifiers", ArgumentSemantic.Copy)]
        NSMutableArray OutOfScopeIdentifiers { get; set; }
    }

    // @interface ODWDiagnosticDataViewer : NSObject
    [BaseType (typeof(NSObject), Name = "ODWDiagnosticDataViewer")]
    public interface DiagnosticDataViewer
    {
        // +(void)initializeViewerWithMachineIdentifier:(NSString * _Nonnull)machineIdentifier;
        [Static]
        [Export ("initializeViewerWithMachineIdentifier:")]
        void InitializeViewerWithMachineIdentifier (string machineIdentifier);

        // +(void)enableRemoteViewer:(NSString * _Nonnull)endpoint completionWithResult:(void (^ _Nonnull)(_Bool))completion;
        [Static]
        [Export ("enableRemoteViewer:completionWithResult:")]
        void EnableRemoteViewer (string endpoint, Action<bool> completion);

        // +(_Bool)enableRemoteViewer:(NSString * _Nonnull)endpoint;
        [Static]
        [Export ("enableRemoteViewer:")]
        bool EnableRemoteViewer (string endpoint);

        // +(void)disableViewer:(void (^ _Nonnull)(_Bool))completion;
        [Static]
        [Export ("disableViewer:")]
        void DisableViewer (Action<bool> completion);

        // +(_Bool)viewerEnabled;
        [Static]
        [Export ("viewerEnabled")]
        bool ViewerEnabled { get; }

        // +(NSString * _Nullable)currentEndpoint;
        [Static]
        [NullAllowed, Export ("currentEndpoint")]
        string CurrentEndpoint { get; }

        // +(void)registerOnDisableNotification:(void (^ _Nonnull)(void))callback;
        [Static]
        [Export ("registerOnDisableNotification:")]
        void RegisterOnDisableNotification (Action callback);
    }

    // @interface ODWEventProperties : NSObject
    [BaseType (typeof(NSObject), Name = "ODWEventProperties")]
    [DisableDefaultCtor]
    public interface EventProperties
    {
        // @property (readwrite, copy, nonatomic) NSString * _Nonnull name;
        [Export ("name")]
        string Name { get; set; }

        // @property (readwrite, nonatomic) ODWEventPriority priority;
        [Export ("priority", ArgumentSemantic.Assign)]
        EventPriority Priority { get; set; }

        // @property (readonly, copy, nonatomic) NSDictionary<NSString *,id> * _Nonnull properties;
        [Export ("properties", ArgumentSemantic.Copy)]
        NSDictionary<NSString, NSObject> Properties { get; }

        // @property (readonly, copy, nonatomic) NSDictionary<NSString *,NSNumber *> * _Nonnull piiTags;
        [Export ("piiTags", ArgumentSemantic.Copy)]
        NSDictionary<NSString, NSNumber> PiiTags { get; }

        // -(instancetype _Nonnull)initWithName:(NSString * _Nonnull)name;
        [Export ("initWithName:")]
        IntPtr Constructor (string name);

        // -(instancetype _Nonnull)initWithName:(NSString * _Nonnull)name properties:(NSDictionary<NSString *,id> * _Nonnull)properties;
        [Export ("initWithName:properties:")]
        IntPtr Constructor (string name, NSDictionary<NSString, NSObject> properties);

        // -(instancetype _Nonnull)initWithName:(NSString * _Nonnull)name properties:(NSDictionary<NSString *,id> * _Nonnull)properties piiTags:(NSDictionary<NSString *,NSNumber *> * _Nonnull)piiTags __attribute__((objc_designated_initializer));
        [Export ("initWithName:properties:piiTags:")]
        [DesignatedInitializer]
        IntPtr Constructor (string name, NSDictionary<NSString, NSObject> properties, NSDictionary<NSString, NSNumber> piiTags);

        // -(void)setProperty:(NSString * _Nonnull)name withValue:(id _Nonnull)value;
        [Export ("setProperty:withValue:")]
        void SetProperty (string name, string value);

        // -(void)setProperty:(NSString * _Nonnull)name withValue:(id _Nonnull)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withValue:withPiiKind:")]
        void SetProperty (string name, string value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withDoubleValue:(double)value;
        [Export ("setProperty:withDoubleValue:")]
        void SetProperty (string name, double value);

        // -(void)setProperty:(NSString * _Nonnull)name withDoubleValue:(double)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withDoubleValue:withPiiKind:")]
        void SetProperty (string name, double value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withInt64Value:(int64_t)value;
        [Export ("setProperty:withInt64Value:")]
        void SetProperty (string name, long value);

        // -(void)setProperty:(NSString * _Nonnull)name withInt64Value:(int64_t)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withInt64Value:withPiiKind:")]
        void SetProperty (string name, long value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withUInt8Value:(uint8_t)value;
        [Export ("setProperty:withUInt8Value:")]
        void SetProperty (string name, byte value);

        // -(void)setProperty:(NSString * _Nonnull)name withUInt8Value:(uint8_t)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withUInt8Value:withPiiKind:")]
        void SetProperty (string name, byte value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withUInt64Value:(uint64_t)value;
        [Export ("setProperty:withUInt64Value:")]
        void SetProperty (string name, ulong value);

        // -(void)setProperty:(NSString * _Nonnull)name withUInt64Value:(uint64_t)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withUInt64Value:withPiiKind:")]
        void SetProperty (string name, ulong value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withBoolValue:(BOOL)value;
        [Export ("setProperty:withBoolValue:")]
        void SetProperty (string name, bool value);

        // -(void)setProperty:(NSString * _Nonnull)name withBoolValue:(BOOL)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withBoolValue:withPiiKind:")]
        void SetProperty (string name, bool value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withUUIDValue:(NSUUID * _Nonnull)value;
        [Export ("setProperty:withUUIDValue:")]
        void SetProperty (string name, NSUuid value);

        // -(void)setProperty:(NSString * _Nonnull)name withUUIDValue:(NSUUID * _Nonnull)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withUUIDValue:withPiiKind:")]
        void SetProperty (string name, NSUuid value, PiiKind piiKind);

        // -(void)setProperty:(NSString * _Nonnull)name withDateValue:(NSDate * _Nonnull)value;
        [Export ("setProperty:withDateValue:")]
        void SetProperty (string name, NSDate value);

        // -(void)setPrivacyMetadata:(ODWPrivacyDataType)privTags withODWDiagLevel:(ODWDiagLevel)privLevel;
        [Export ("setPrivacyMetadata:withODWDiagLevel:")]
        void SetPrivacyMetadata (PrivacyDataType privTags, DiagLevel privLevel);

        // -(void)setProperty:(NSString * _Nonnull)name withDateValue:(NSDate * _Nonnull)value withPiiKind:(ODWPiiKind)piiKind;
        [Export ("setProperty:withDateValue:withPiiKind:")]
        void SetProperty (string name, NSDate value, PiiKind piiKind);
    }

    // @interface ODWLogConfiguration : NSObject
    [BaseType(typeof(NSObject), Name = "ODWLogConfiguration")]
    [Protocol]
    public interface LogConfiguration
    {
        // +(NSString * _Nullable)eventCollectorUri;
        // +(void)setEventCollectorUri:(NSString * _Nonnull)eventCollectorUri;
        [Static]
        [NullAllowed, Export("eventCollectorUri")]
        string EventCollectorUri { get; set; }

        // +(uint64_t)cacheMemorySizeLimitInBytes;
        // +(void)setCacheMemorySizeLimitInBytes:(uint64_t)cacheMemorySizeLimitInBytes;
        [Static]
        [Export("cacheMemorySizeLimitInBytes")]
        ulong CacheMemorySizeLimitInBytes { get; set; }

        // +(uint64_t)cacheFileSizeLimitInBytes;
        // +(void)setCacheFileSizeLimitInBytes:(uint64_t)cacheFileSizeLimitInBytes;
        [Static]
        [Export("cacheFileSizeLimitInBytes")]
        ulong CacheFileSizeLimitInBytes { get; set; }

        // +(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec;
        [Static]
        [Export("setMaxTeardownUploadTimeInSec:")]
        void SetMaxTeardownUploadTimeInSec(int maxTeardownUploadTimeInSec);

        // +(void)setTraceLevel:(int)TraceLevel;
        [Static]
        [Export("setTraceLevel:")]
        void SetTraceLevel(int TraceLevel);

        // +(_Bool)enableTrace;
        // +(void)setEnableTrace:(_Bool)enableTrace;
        [Static]
        [Export("enableTrace")]
        bool EnableTrace { get; set; }

        // +(_Bool)enableConsoleLogging;
        // +(void)setEnableConsoleLogging:(_Bool)enableConsoleLogging;
        [Static]
        [Export("enableConsoleLogging")]
        bool EnableConsoleLogging { get; set; }

        // +(_Bool)surfaceCppExceptions;
        // +(void)setSurfaceCppExceptions:(_Bool)surfaceCppExceptions;
        [Static]
        [Export("surfaceCppExceptions")]
        bool SurfaceCppExceptions { get; set; }

        // +(_Bool)enableSessionReset;
        // +(void)setEnableSessionReset:(_Bool)enableSessionReset;
        [Static]
        [Export("enableSessionReset")]
        bool EnableSessionReset { get; set; }

        // +(NSString * _Nullable)cacheFilePath;
        // +(void)setCacheFilePath:(NSString * _Nonnull)cacheFilePath;
        [Static]
        [NullAllowed, Export("cacheFilePath")]
        string CacheFilePath { get; set; }
    }

    // @interface ODWSemanticContext : NSObject
    [BaseType (typeof(NSObject), Name = "ODWSemanticContext")]
    [Protocol]
    public interface SemanticContext
    {
        // -(void)setAppId:(NSString * _Nonnull)appId;
        [Export ("setAppId:")]
        void SetAppId (string appId);

        // -(void)setAppVersion:(NSString * _Nonnull)appVersion;
        [Export ("setAppVersion:")]
        void SetAppVersion (string appVersion);

        // -(void)setAppLanguage:(NSString * _Nonnull)appLanguage;
        [Export ("setAppLanguage:")]
        void SetAppLanguage (string appLanguage);

        // -(void)setUserId:(NSString * _Nonnull)userId;
        [Export ("setUserId:")]
        void SetUserId (string userId);

        // -(void)setUserId:(NSString * _Nonnull)userId piiKind:(enum ODWPiiKind)pii;
        [Export ("setUserId:piiKind:")]
        void SetUserId (string userId, PiiKind pii);

        // -(void)setDeviceId:(NSString * _Nonnull)deviceId;
        [Export ("setDeviceId:")]
        void SetDeviceId (string deviceId);

        // -(void)setUserTimeZone:(NSString * _Nonnull)userTimeZone;
        [Export ("setUserTimeZone:")]
        void SetUserTimeZone (string userTimeZone);

        // -(void)setUserAdvertisingId:(NSString * _Nonnull)userAdvertisingId;
        [Export ("setUserAdvertisingId:")]
        void SetUserAdvertisingId (string userAdvertisingId);

        // -(void)setAppExperimentIds:(NSString * _Nonnull)experimentIds;
        [Export ("setAppExperimentIds:")]
        void SetAppExperimentIds (string experimentIds);

        // -(void)setAppExperimentIds:(NSString * _Nonnull)experimentIds forEvent:(NSString * _Nonnull)eventName;
        [Export ("setAppExperimentIds:forEvent:")]
        void SetAppExperimentIds (string experimentIds, string eventName);

        // -(void)setAppExperimentETag:(NSString * _Nonnull)eTag;
        [Export ("setAppExperimentETag:")]
        void SetAppExperimentETag (string eTag);

        // -(void)setAppExperimentImpressionId:(NSString * _Nonnull)impressionId;
        [Export ("setAppExperimentImpressionId:")]
        void SetAppExperimentImpressionId (string impressionId);
    }

    // @interface ODWLogger : NSObject
    [BaseType (typeof(NSObject), Name = "ODWLogger")]
    [Protocol]
    public interface Logger
    {
        // -(void)logEventWithName:(NSString * _Nonnull)name;
        [Export ("logEventWithName:")]
        void LogEvent (string name);

        // -(void)logEventWithEventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logEventWithEventProperties:")]
        void LogEvent (EventProperties properties);

        // -(void)logFailureWithSignature:(NSString * _Nonnull)signature detail:(NSString * _Nonnull)detail eventproperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logFailureWithSignature:detail:eventproperties:")]
        void LogFailure (string signature, string detail, EventProperties properties);

        // -(void)logFailureWithSignature:(NSString * _Nonnull)signature detail:(NSString * _Nonnull)detail category:(NSString * _Nonnull)category id:(NSString * _Nonnull)identifier eventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logFailureWithSignature:detail:category:id:eventProperties:")]
        void LogFailure (string signature, string detail, string category, string identifier, EventProperties properties);

        // -(void)logPageViewWithId:(NSString * _Nonnull)identifier pageName:(NSString * _Nonnull)pageName eventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logPageViewWithId:pageName:eventProperties:")]
        void LogPageView (string identifier, string pageName, EventProperties properties);

        // -(void)logPageViewWithId:(NSString * _Nonnull)identifier pageName:(NSString * _Nonnull)pageName category:(NSString * _Nonnull)category uri:(NSString * _Nonnull)uri referrerUri:(NSString * _Nonnull)referrerUri eventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logPageViewWithId:pageName:category:uri:referrerUri:eventProperties:")]
        void LogPageView (string identifier, string pageName, string category, string uri, string referrerUri, EventProperties properties);

        // -(void)logTraceWithTraceLevel:(enum ODWTraceLevel)traceLevel message:(NSString * _Nonnull)message eventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logTraceWithTraceLevel:message:eventProperties:")]
        void LogTrace (TraceLevel traceLevel, string message, EventProperties properties);

        // -(void)logSessionWithState:(enum ODWSessionState)state eventProperties:(ODWEventProperties * _Nonnull)properties;
        [Export ("logSessionWithState:eventProperties:")]
        void LogSession (SessionState state, EventProperties properties);

        // -(void)initializePrivacyGuardWithODWCommonDataContext:(ODWCommonDataContext * _Nonnull)commonDataContextsObject;
        [Export ("initializePrivacyGuardWithODWCommonDataContext:")]
        void InitializePrivacyGuard (CommonDataContext commonDataContextsObject);

        // -(void)setContextWithName:(NSString * _Nonnull)name stringValue:(NSString * _Nonnull)value;
        [Export ("setContextWithName:stringValue:")]
        void SetContext (string name, string value);

        // -(void)setContextWithName:(NSString * _Nonnull)name stringValue:(NSString * _Nonnull)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:stringValue:piiKind:")]
        void SetContext (string name, string value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name boolValue:(BOOL)value;
        [Export ("setContextWithName:boolValue:")]
        void SetContext (string name, bool value);

        // -(void)setContextWithName:(NSString * _Nonnull)name boolValue:(BOOL)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:boolValue:piiKind:")]
        void SetContext (string name, bool value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name dateValue:(NSDate * _Nonnull)value;
        [Export ("setContextWithName:dateValue:")]
        void SetContext (string name, NSDate value);

        // -(void)setContextWithName:(NSString * _Nonnull)name dateValue:(NSDate * _Nonnull)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:dateValue:piiKind:")]
        void SetContext (string name, NSDate value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name doubleValue:(double)value;
        [Export ("setContextWithName:doubleValue:")]
        void SetContext (string name, double value);

        // -(void)setContextWithName:(NSString * _Nonnull)name doubleValue:(double)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:doubleValue:piiKind:")]
        void SetContext (string name, double value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name int64Value:(int64_t)value;
        [Export ("setContextWithName:int64Value:")]
        void SetContext (string name, long value);

        // -(void)setContextWithName:(NSString * _Nonnull)name int64Value:(int64_t)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:int64Value:piiKind:")]
        void SetContext (string name, long value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name int32Value:(int32_t)value;
        [Export ("setContextWithName:int32Value:")]
        void SetContext (string name, int value);

        // -(void)setContextWithName:(NSString * _Nonnull)name int32Value:(int32_t)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:int32Value:piiKind:")]
        void SetContext (string name, int value, PiiKind piiKind);

        // -(void)setContextWithName:(NSString * _Nonnull)name UUIDValue:(NSUUID * _Nonnull)value;
        [Export ("setContextWithName:UUIDValue:")]
        void SetContext (string name, NSUuid value);

        // -(void)setContextWithName:(NSString * _Nonnull)name UUIDValue:(NSUUID * _Nonnull)value piiKind:(enum ODWPiiKind)piiKind;
        [Export ("setContextWithName:UUIDValue:piiKind:")]
        void SetContext (string name, NSUuid value, PiiKind piiKind);

        // @property (readonly, nonatomic, strong) ODWSemanticContext * _Nonnull semanticContext;
        [Export ("semanticContext", ArgumentSemantic.Strong)]
        SemanticContext SemanticContext { get; }
    }

    // @interface ODWLogManager : NSObject
    [BaseType (typeof(NSObject), Name = "ODWLogManager")]
    [Protocol]
    public interface LogManager
    {
        // +(ODWLogger * _Nullable)loggerWithTenant:(NSString * _Nonnull)tenantToken;
        [Static]
        [Export ("initForTenant:")]
        [return: NullAllowed]
        Logger InitializeLogger(string tenantToken);

        // +(ODWLogger * _Nullable)initForTenant:(NSString * _Nonnull)tenantToken withConfig:(NSDictionary * _Nullable)config;
        [Static]
        [Export ("initForTenant:withConfig:")]
        [return: NullAllowed]
        Logger InitializeLogger (string tenantToken, [NullAllowed] NSDictionary config);

        // +(void)uploadNow;
        [Static]
        [Export ("uploadNow")]
        void UploadNow ();

        // +(ODWStatus)flush;
        [Static]
        [Export("flush")]
        Status Flush();

        // +(void)setTransmissionProfile:(ODWTransmissionProfile)profile;
        [Static]
        [Export ("setTransmissionProfile:")]
        void SetTransmissionProfile (TransmissionProfile profile);

        // +(void)pauseTransmission;
        [Static]
        [Export ("pauseTransmission")]
        void PauseTransmission ();

        // +(void)resumeTransmission;
        [Static]
        [Export ("resumeTransmission")]
        void ResumeTransmission ();

        // +(ODWStatus)flushAndTeardown;
        [Static]
        [Export ("flushAndTeardown")]
        Status FlushAndTeardown ();

        // +(void)resetTransmitProfiles;
        [Static]
        [Export ("resetTransmitProfiles")]
        void ResetTransmitProfiles ();

        // +(void)setContextWithName:(NSString * _Nonnull)name stringValue:(NSString * _Nonnull)value;
        [Static]
        [Export ("setContextWithName:stringValue:")]
        void SetContext (string name, string value);

        // +(void)setContextWithName:(NSString * _Nonnull)name stringValue:(NSString * _Nonnull)value piiKind:(enum ODWPiiKind)piiKind;
        [Static]
        [Export ("setContextWithName:stringValue:piiKind:")]
        void SetContext (string name, string value, PiiKind piiKind);
    }

    // @interface ODWPrivacyGuard : NSObject
    [BaseType (typeof(NSObject), Name = "ODWPrivacyGuard")]
    [Protocol]
    public interface PrivacyGuard
    {
        // +(_Bool)enabled;
        // +(void)setEnabled:(_Bool)enabled;
        [Static]
        [Export ("enabled")]
        bool Enabled { get; set; }

        // +(void)appendCommonDataContext:(ODWCommonDataContext * _Nonnull)freshCommonDataContext;
        [Static]
        [Export ("appendCommonDataContext:")]
        void AppendCommonDataContext (CommonDataContext freshCommonDataContext);

        // +(void)addIgnoredConcern:(NSString * _Nonnull)EventName withNSString:(NSString * _Nonnull)FieldName withODWDataConcernType:(ODWDataConcernType)IgnoredConcern;
        [Static]
        [Export ("addIgnoredConcern:withNSString:withODWDataConcernType:")]
        void AddIgnoredConcern (string EventName, string FieldName, DataConcernType IgnoredConcern);
    }
}

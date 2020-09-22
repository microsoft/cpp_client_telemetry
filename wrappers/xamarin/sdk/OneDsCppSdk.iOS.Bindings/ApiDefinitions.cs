using System;
using Foundation;
using ObjCRuntime;

namespace Microsoft.Applications.Events
{
	// @interface ODWDiagnosticDataViewer : NSObject
	[BaseType (typeof(NSObject), Name = "ODWDiagnosticDataViewer")]
	public interface DiagnosticDataViewer
	{
		// +(void)initializeViewerWithMachineIdentifier:(NSString * _Nonnull __strong)machineIdentifier;
		[Static]
		[Export ("initializeViewerWithMachineIdentifier:")]
		void InitializeViewerWithMachineIdentifier (string machineIdentifier);

		// +(void)enableRemoteViewer:(NSString * _Nonnull __strong)endpoint completionWithResult:(void (^ _Nonnull __strong)(_Bool))completion;
		[Static]
		[Export ("enableRemoteViewer:completionWithResult:")]
		void EnableRemoteViewer (string endpoint, Action<bool> completion);

		// +(_Bool)enableRemoteViewer:(NSString * _Nonnull __strong)endpoint;
		[Static]
		[Export ("enableRemoteViewer:")]
		bool EnableRemoteViewer (string endpoint);

		// +(void)disableViewer:(void (^ _Nonnull __strong)(_Bool))completion;
		[Static]
		[Export ("disableViewer:")]
		void DisableViewer (Action<bool> completion);

		// +(_Bool)viewerEnabled;
		[Static]
		[Export ("viewerEnabled")]
		//[Verify (MethodToProperty)]
		bool ViewerEnabled { get; }

		// +(NSString * _Nullable)currentEndpoint;
		[Static]
		[NullAllowed, Export ("currentEndpoint")]
		//[Verify (MethodToProperty)]
		string CurrentEndpoint { get; }

		// +(void)registerOnDisableNotification:(void (^ _Nonnull __strong)(void))callback;
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

		// -(instancetype _Nonnull)initWithName:(NSString * _Nonnull __strong)name __attribute__((ns_consumes_self)) __attribute__((ns_returns_retained));
		[Export ("initWithName:")]
		IntPtr Constructor (string name);

		// -(instancetype _Nonnull)initWithName:(NSString * _Nonnull __strong)name properties:(NSDictionary<NSString *,id> * _Nonnull __strong)properties __attribute__((ns_consumes_self)) __attribute__((ns_returns_retained));
		[Export ("initWithName:properties:")]
		IntPtr Constructor (string name, NSDictionary<NSString, NSObject> properties);

		// -(instancetype _Nonnull)initWithName:(NSString * _Nonnull __strong)name properties:(NSDictionary<NSString *,id> * _Nonnull __strong)properties piiTags:(NSDictionary<NSString *,NSNumber *> * _Nonnull __strong)piiTags __attribute__((objc_designated_initializer)) __attribute__((ns_consumes_self)) __attribute__((ns_returns_retained));
		[Export ("initWithName:properties:piiTags:")]
		[DesignatedInitializer]
		IntPtr Constructor (string name, NSDictionary<NSString, NSObject> properties, NSDictionary<NSString, NSNumber> piiTags);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withValue:(id  _Nonnull __strong)value;
		[Export ("setProperty:withValue:")]
		void SetProperty (string name, string value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withValue:(id  _Nonnull __strong)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withValue:withPiiKind:")]
		void SetProperty (string name, string value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withDoubleValue:(double)value;
		[Export ("setProperty:withDoubleValue:")]
		void SetProperty (string name, double value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withDoubleValue:(double)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withDoubleValue:withPiiKind:")]
		void SetProperty (string name, double value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withInt64Value:(int64_t)value;
		[Export ("setProperty:withInt64Value:")]
		void SetProperty (string name, long value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withInt64Value:(int64_t)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withInt64Value:withPiiKind:")]
		void SetProperty (string name, long value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUInt8Value:(uint8_t)value;
		[Export ("setProperty:withUInt8Value:")]
		void SetProperty (string name, byte value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUInt8Value:(uint8_t)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withUInt8Value:withPiiKind:")]
		void SetProperty (string name, byte value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUInt64Value:(uint64_t)value;
		[Export ("setProperty:withUInt64Value:")]
		void SetProperty (string name, ulong value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUInt64Value:(uint64_t)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withUInt64Value:withPiiKind:")]
		void SetProperty (string name, ulong value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withBoolValue:(BOOL)value;
		[Export ("setProperty:withBoolValue:")]
		void SetProperty (string name, bool value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withBoolValue:(BOOL)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withBoolValue:withPiiKind:")]
		void SetProperty (string name, bool value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUUIDValue:(NSUUID * _Nonnull __strong)value;
		[Export ("setProperty:withUUIDValue:")]
		void SetProperty (string name, NSUuid value);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withUUIDValue:(NSUUID * _Nonnull __strong)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withUUIDValue:withPiiKind:")]
		void SetProperty (string name, NSUuid value, PiiKind piiKind);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withDateValue:(NSDate * _Nonnull __strong)value;
		[Export ("setProperty:withDateValue:")]
		void SetProperty (string name, NSDate value);

		// -(void)setPrivacyMetadata:(ODWPrivacyDataType)privTags withODWDiagLevel:(ODWDiagLevel)privLevel;
		[Export ("setPrivacyMetadata:withODWDiagLevel:")]
		void SetPrivacyMetadata (ulong privTags, DiagLevel privLevel);

		// -(void)setProperty:(NSString * _Nonnull __strong)name withDateValue:(NSDate * _Nonnull __strong)value withPiiKind:(ODWPiiKind)piiKind;
		[Export ("setProperty:withDateValue:withPiiKind:")]
		void SetProperty (string name, NSDate value, PiiKind piiKind);
	}

	// @interface ODWLogConfiguration : NSObject
	[BaseType (typeof(NSObject), Name = "ODWLogConfiguration")]
	[Protocol]
	public interface LogConfiguration
	{
		// +(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec;
		[Static]
		[Export ("setMaxTeardownUploadTimeInSec:")]
		void SetMaxTeardownUploadTimeInSec (int maxTeardownUploadTimeInSec);

		// +(void)setTraceLevel:(int)TraceLevel;
		[Static]
		[Export ("setTraceLevel:")]
		void SetTraceLevel (int TraceLevel);

		// +(_Bool)enableTrace;
		// +(void)setEnableTrace:(_Bool)enableTrace;
		[Static]
		[Export ("enableTrace")]
		bool EnableTrace { get; set; }

		// +(_Bool)surfaceCppExceptions;
		// +(void)setSurfaceCppExceptions:(_Bool)surfaceCppExceptions;
		[Static]
		[Export ("surfaceCppExceptions")]
		bool SurfaceCppExceptions { get; set; }
	}

	// @interface ODWSemanticContext : NSObject
	[BaseType (typeof(NSObject), Name = "ODWSemanticContext")]
	[Protocol]
	public interface SemanticContext
	{
		// -(void)setAppId:(NSString * _Nonnull __strong)appId;
		[Export ("setAppId:")]
		void SetAppId (string appId);

		// -(void)setUserId:(NSString * _Nonnull __strong)userId;
		[Export ("setUserId:")]
		void SetUserId (string userId);

		// -(void)setUserAdvertisingId:(NSString * _Nonnull __strong)userAdvertisingId;
		[Export ("setUserAdvertisingId:")]
		void SetUserAdvertisingId (string userAdvertisingId);

		// -(void)setAppExperimentIds:(NSString * _Nonnull __strong)experimentIds;
		[Export ("setAppExperimentIds:")]
		void SetAppExperimentIds (string experimentIds);

		// -(void)setAppExperimentIds:(NSString * _Nonnull __strong)experimentIds forEvent:(NSString * _Nonnull __strong)eventName;
		[Export ("setAppExperimentIds:forEvent:")]
		void SetAppExperimentIds (string experimentIds, string eventName);

		// -(void)setAppExperimentETag:(NSString * _Nonnull __strong)eTag;
		[Export ("setAppExperimentETag:")]
		void SetAppExperimentETag (string eTag);

		// -(void)setAppExperimentImpressionId:(NSString * _Nonnull __strong)impressionId;
		[Export ("setAppExperimentImpressionId:")]
		void SetAppExperimentImpressionId (string impressionId);
	}

	// @interface ODWLogger : NSObject
	[BaseType (typeof(NSObject), Name = "ODWLogger")]
	[Protocol]
	public interface Logger
	{
		// -(void)logEventWithName:(NSString * _Nonnull __strong)name;
		[Export ("logEventWithName:")]
		void LogEvent (string name);

		// -(void)logEventWithEventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logEventWithEventProperties:")]
		void LogEvent (EventProperties properties);

		// -(void)logFailureWithSignature:(NSString * _Nonnull __strong)signature detail:(NSString * _Nonnull __strong)detail eventproperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logFailureWithSignature:detail:eventproperties:")]
		void LogFailure (string signature, string detail, EventProperties properties);

		// -(void)logFailureWithSignature:(NSString * _Nonnull __strong)signature detail:(NSString * _Nonnull __strong)detail category:(NSString * _Nonnull __strong)category id:(NSString * _Nonnull __strong)identifier eventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logFailureWithSignature:detail:category:id:eventProperties:")]
		void LogFailure (string signature, string detail, string category, string identifier, EventProperties properties);

		// -(void)logPageViewWithId:(NSString * _Nonnull __strong)identifier pageName:(NSString * _Nonnull __strong)pageName eventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logPageViewWithId:pageName:eventProperties:")]
		void LogPageView (string identifier, string pageName, EventProperties properties);

		// -(void)logPageViewWithId:(NSString * _Nonnull __strong)identifier pageName:(NSString * _Nonnull __strong)pageName category:(NSString * _Nonnull __strong)category uri:(NSString * _Nonnull __strong)uri referrerUri:(NSString * _Nonnull __strong)referrerUri eventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logPageViewWithId:pageName:category:uri:referrerUri:eventProperties:")]
		void LogPageView (string identifier, string pageName, string category, string uri, string referrerUri, EventProperties properties);

		// -(void)logTraceWithTraceLevel:(enum ODWTraceLevel)traceLevel message:(NSString * _Nonnull __strong)message eventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logTraceWithTraceLevel:message:eventProperties:")]
		void LogTrace (TraceLevel traceLevel, string message, EventProperties properties);

		// -(void)logSessionWithState:(enum ODWSessionState)state eventProperties:(ODWEventProperties * _Nonnull __strong)properties;
		[Export ("logSessionWithState:eventProperties:")]
		void LogSession (SessionState state, EventProperties properties);

		// -(void)setContext:(NSString *)name withStringValue:(NSString*) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withStringValue:withPiiKind:")]
		void SetContext(string name, string value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withStringValue:(NSString*) value;
		[Export("setContext:withStringValue:")]
		void SetContext (string name, string value);

		// -(void)setContext:(NSString *)name withDoubleValue:(double) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withDoubleValue:withPiiKind:")]
		void SetContext(string name, double value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withDoubleValue:(double) value;
		[Export("setContext:withDoubleValue:")]
		void SetContext(string name, double value);

		// -(void)setContext:(NSString *)name withInt64Value:(int64_t) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withInt64Value:withPiiKind:")]
		void SetContext(string name, long value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withInt64Value:(int64_t) value;
		[Export("setContext:withInt64Value:")]
		void SetContext(string name, long value);

		// -(void)setContext:(NSString *)name withInt32Value:(int32_t) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withInt32Value:withPiiKind:")]
		void SetContext(string name, int value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withInt32Value:(int32_t) value;
		[Export("setContext:withInt32Value:")]
		void SetContext(string name, int value);

		// -(void)setContext:(NSString *)name withBoolValue:(BOOL) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withBoolValue:withPiiKind:")]
		void SetContext(string name, bool value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withBoolValue:(BOOL) value;
		[Export("setContext:withBoolValue:")]
		void SetContext(string name, bool value);

		// -(void)setContext:(NSString *)name withUUIDValue:(NSUUID *) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withUUIDValue:withPiiKind:")]
		void SetContext(string name, Guid value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withUUIDValue:(NSUUID *) value;
		[Export("setContext:withUUIDValue:")]
		void SetContext(string name, Guid value);

		// -(void)setContext:(NSString *)name withDateValue:(NSDate *) value withPiiKind:(PiiKind) piiKind;
		[Export("setContext:withDateValue:withPiiKind:")]
		void SetContext(string name, NSDate value, PiiKind piiKind);

		// -(void)setContext:(NSString *)name withDateValue:(NSDate *) value;
		[Export("setContext:withDateValue:")]
		void SetContext(string name, NSDate value);

		// @property (readonly, nonatomic, strong) ODWSemanticContext * _Nonnull semanticContext;
		[Export ("semanticContext", ArgumentSemantic.Strong)]
		SemanticContext SemanticContext { get; }
	}

	// @interface ODWLogManager : NSObject
	[BaseType (typeof(NSObject), Name = "ODWLogManager")]
	[Protocol]
	interface LogManager
	{
		// +(ODWLogger * _Nullable)loggerWithTenant:(NSString * _Nonnull __strong)tenantToken;
		[Static]
		[Export("loggerWithTenant:")]
		[return: NullAllowed]
		Logger Initialize(string tenantToken);

		// +(ODWLogger * _Nullable)loggerWithTenant:(NSString * _Nonnull __strong)tenantToken source:(NSString * _Nonnull __strong)source;
		[Static]
		[Export ("loggerWithTenant:source:")]
		[return: NullAllowed]
		Logger LoggerWithTenant (string tenantToken, string source);

		// +(ODWLogger * _Nullable)loggerForSource:(NSString * _Nonnull __strong)source;
		[Static]
		[Export ("loggerForSource:")]
		[return: NullAllowed]
		Logger LoggerForSource (string source);

		// +(void)uploadNow;
		[Static]
		[Export ("uploadNow")]
		void UploadNow ();

		// +(void)flush;
		[Static]
		[Export ("flush")]
		void Flush ();

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

		// +(void)flushAndTeardown;
		[Static]
		[Export ("flushAndTeardown")]
		void FlushAndTeardown ();

		// +(void)resetTransmitProfiles;
		[Static]
		[Export ("resetTransmitProfiles")]
		void ResetTransmitProfiles ();
	}
}

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import "ODWEventProperties.h"

#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief The ODWSemanticContext class manages the inclusion of semantic context values on logged events
*/
@interface ODWSemanticContext : NSObject

/*!
 @brief Specifies an application id to be included with every event
 @param appId A string that contains an application identifier
 */
-(void)setAppId:(NSString *)appId;

/*!
 @brief Set the application version context information of telemetry event.
 @param appVersion Version of the application, retrieved programmatically where possible and is app/platform specific.
 */
-(void) setAppVersion:(nonnull NSString*)appVersion;

/*!
 @brief Set the application language context information of telemetry event.
 @param appLanguage A string that contains the spoken/written language used in the application.
 */
-(void) setAppLanguage:(nonnull NSString*)appLanguage;

/*!
 @brief Specifies a unique user id to be included with every event
 @param userId A string that contains the unique user identifier.
 */
-(void)setUserId:(nonnull NSString *)userId;

/*!
 @brief Set the userId context information of telemetry event.
 @details <b>Note:</b> You can set the pii value to ODWPiiKindNone and ODWPiiKindIdentity only.
 Setting other pii values produces a <i>no-op</i>.
 @param userId Identifier that uniquely identifies a user in the application-specific user namespace.
 @param pii PIIKind of the userId. Set it to PiiKind_None to denote it as non-PII.
 */
-(void) setUserId:(nonnull NSString*)userId
          piiKind:(enum ODWPiiKind)pii;

/*!
 @brief Set the device identifier context information of telemetry event.
 @param deviceId A unique device identifier, retrieved programmatically where possible and is app/platform specific.
 */
-(void) setDeviceId:(nonnull NSString*)deviceId;

/*!
 @brief Set the user time zone context information of telemetry event.
 @param userTimeZone user's time zone relative to UTC, in ISO 8601 time zone format
 */
-(void) setUserTimeZone:(nonnull NSString*)userTimeZone;

/*!
 @brief Specifies a unique user advertising id to be included with every event
 @param userAdvertisingId AA string that contains the unique user advertising identifier.
 */
-(void)setUserAdvertisingId:(nonnull NSString *)userAdvertisingId;

/*!
 @brief Sets the experimentation IDs for determining the deployment configuration.
 @param experimentIds A string that contains the experimentation IDs.
 */
-(void)setAppExperimentIds:(nonnull NSString*)experimentIds;

/*!
 @brief Sets the experimentation IDs for the specified telemetry event.
 @param experimentIds A string that contains the experimentation IDs.
 @param eventName A string that contains the name of the event.
 */
-(void)setAppExperimentIds:(nonnull NSString*)experimentIds
                  forEvent:(nonnull NSString*)eventName;

/*!
 @brief Sets the experiment tag (experiment configuration) context information for telemetry events.
 <b>Note:</b> This method removes any previously stored experiment IDs that were set using setAppExperimentETag.
 @param eTag A string that contains the ETag which is a hash of the set of experiments.
 */
-(void)setAppExperimentETag:(nonnull NSString *)eTag;

/*!
 @brief Sets the impression ID (an identifier of the currently running flights) for an experiment.
 @details Calling this method removes the previously stored experimentation IDs and flight IDs.
 @param impressionId A string that contains the impression ID for the currently active configuration.
 */
-(void)setAppExperimentImpressionId:(nonnull NSString*)impressionId;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"

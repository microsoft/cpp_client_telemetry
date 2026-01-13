//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief Represents a metadata provider for privacy concern events.
 */
@interface ODWPrivacyConcernMetadataProvider : NSObject

/*!
 @brief Get the database name.
 @param record The record for which to retrieve the database name.
 @return A string representing the database name.
 */
- (NSString *)getDatabaseNameForRecord:(id)record;

/*!
 @brief Get the server name.
 @param record The record for which to retrieve the server name.
 @return A string representing the server name.
 */
- (NSString *)getServerNameForRecord:(id)record;

/*!
 @brief Get the event locator name.
 @param record The record for which to retrieve the event locator name.
 @return A string representing the event locator name.
 */
- (NSString *)getEventLocatorNameForRecord:(id)record;

/*!
 @brief Get the event locator value.
 @param record The record for which to retrieve the event locator value.
 @return A string representing the event locator value.
 */
- (NSString *)getEventLocatorValueForRecord:(id)record;

/*!
 @brief Get the override for the privacy guard event time.
 @param record The record for which to retrieve the event time override.
 @return An integer representing the event time override.
 */
- (int64_t)getPrivacyGuardEventTimeOverrideForRecord:(id)record;

/*!
 @brief Check if the record should be ignored.
 @param record The record to check.
 @return A boolean indicating whether the record should be ignored.
 */
- (BOOL)getShouldIgnoreOverrideForRecord:(id)record;

/*!
 @brief Get the associated tenant.
 @param record The record for which to retrieve the associated tenant.
 @return A string representing the associated tenant.
 */
- (NSString *)getAssociatedTenantForRecord:(id)record;

/*!
 @brief Get the environment.
 @param record The record for which to retrieve the environment.
 @return A string representing the environment.
 */
- (NSString *)getEnvironmentForRecord:(id)record;

/*!
 @brief Get the metadata.
 @param record The record for which to retrieve the metadata.
 @return A string representing the metadata.
 */
- (NSString *)getMetadataForRecord:(id)record;

@end

NS_ASSUME_NONNULL_END

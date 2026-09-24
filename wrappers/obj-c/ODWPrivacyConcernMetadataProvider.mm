
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#import <Foundation/Foundation.h>
#import "ODWPrivacyConcernMetadataProvider.h"

/*!
 @brief Represents a metadata provider for privacy concern events.
 */
@implementation ODWPrivacyConcernMetadataProvider : NSObject

- (NSString *)getDatabaseNameForRecord:(id)record {
    return @""; // Default implementation
}

- (NSString *)getServerNameForRecord:(id)record {
    return @""; // Default implementation
}

- (NSString *)getEventLocatorNameForRecord:(id)record {
    return @"BaseType"; // Default implementation
}

- (NSString *)getEventLocatorValueForRecord:(id)record {
    return [record baseType]; // Example assuming `record` has a `baseType` property
}

- (int64_t)getPrivacyGuardEventTimeOverrideForRecord:(id)record {
    return [record time]; // Example assuming `record` has a `time` property
}

- (BOOL)getShouldIgnoreOverrideForRecord:(id)record {
    return NO; // Default implementation
}

- (NSString *)getAssociatedTenantForRecord:(id)record {
    return [record iKey]; // Example assuming `record` has an `iKey` property
}

- (NSString *)getEnvironmentForRecord:(id)record {
    return @""; // Default implementation
}

- (NSString *)getMetadataForRecord:(id)record {
    return @""; // Default implementation
}

@end

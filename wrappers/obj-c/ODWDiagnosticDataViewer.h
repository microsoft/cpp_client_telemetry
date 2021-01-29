//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 The <b>ODWDiagnosticDataViewer</b> class represents Diagnostic Data Viewer Hook.
*/
@interface ODWDiagnosticDataViewer : NSObject

#pragma mark Initialization methods

/*!
 @brief Initializes Data Viewer with an specified machine identifier.
 @param machineIdentifier  A string that contains the machine identifier.
 */
+(void)initializeViewerWithMachineIdentifier:(NSString *)machineIdentifier;

#pragma mark Behavior methods
/*!
 @brief Enables Data Viewer.
 @param endpoint A string that contains endpoint to route events.
 @param completion Code to execute when enable is completed. <b>Note:</b> This value can be null.
 */
+(void)enableRemoteViewer:(NSString *)endpoint
	 completionWithResult:(void(^)(bool result))completion;

/*!
 @brief Enables Data Viewer.
 @param endpoint A string that contains endpoint to route events.
 */
+(bool)enableRemoteViewer:(NSString *)endpoint;

/*!
 @brief Disables Data Viewer.
 @param completion Code to execute when disable is completed. <b>Note:</b> This value can be null.
 */
+(void)disableViewer:(void(^)(bool result))completion;

/*!
 @brief Disables Data Viewer.
 */
+(bool)disableViewer;

/*!
 @brief Returns if Data Viewer is enabled or not.
 @return True if viewer is enabled
 */
+(bool)viewerEnabled;

/*!
 @brief Returns current endpoint if it is set, empty string otherwise.
 @return Returns current endpoint if it is set, empty string otherwise.
 */
+(nullable NSString *)currentEndpoint;

/*!
 @brief Sets callback for OnDisableNotification event.
 @param callback Code to execute when OnDisableNotification event occurrs.
 */
+(void)registerOnDisableNotification:(void(^)(void))callback;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"

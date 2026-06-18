//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// Umbrella header for the Obj-C surface vended by MATTelemetry.xcframework.
//
// build-xcframework.sh flattens the ODW*.h headers into the framework's
// Headers/ directory, so these are imported by bare name (not by the
// ../../obj-c/ relative path the in-repo Swift bridging header uses).
//
// build-xcframework.sh copies this template and appends imports for optional
// module headers only when those modules were actually built into the binary.

#import <Foundation/Foundation.h>

#import "ODWCommonDataContext.h"
#import "ODWEventProperties.h"
#import "ODWLogConfiguration.h"
#import "ODWLogger.h"
#import "ODWLogManager.h"
#import "ODWPrivacyGuardInitConfig.h"
#import "ODWSanitizerInitConfig.h"
#import "ODWSemanticContext.h"

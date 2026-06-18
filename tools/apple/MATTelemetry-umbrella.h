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
// Keep this list in sync with:
//   * the ODW*.h headers copied by tools/apple/build-xcframework.sh, and
//   * the Swift sources compiled in wrappers/swift/Sources/OneDSSwift.
//
// TODO(validate on macOS): confirm the ODW headers reference each other by bare
// name (or adjust the copy step) so the flattened layout compiles cleanly.

#import <Foundation/Foundation.h>

#import "ODWDiagnosticDataViewer.h"
#import "ODWEventProperties.h"
#import "ODWLogConfiguration.h"
#import "ODWLogger.h"
#import "ODWLogManager.h"
#import "ODWPrivacyGuard.h"
#import "ODWPrivacyGuardInitConfig.h"
#import "ODWSanitizer.h"
#import "ODWSanitizerInitConfig.h"
#import "ODWSemanticContext.h"

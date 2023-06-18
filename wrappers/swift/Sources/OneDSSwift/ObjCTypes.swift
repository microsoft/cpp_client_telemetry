//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Contains alias for the types declared in the ObjC header files to make them available
/// as part of the swift package module.
/// To avoid clients not have to import ObjCModule explicitly.

/// Check corresponding header file for the doc of each type.

import ObjCModule

// ODWEventProperties.h
public typealias PIIKind = ODWPiiKind
public typealias PrivacyDataType = ODWPrivacyDataType
public typealias DiagnosticLevel = ODWDiagLevel

// ODWLogger.h
public typealias TraceLevel = ODWTraceLevel
public typealias SessionState = ODWSessionState

// ODWLogManager.h
public typealias TransmissionProfile = ODWTransmissionProfile
public typealias FlushStatus = ODWStatus

// ODWPrivacyGuard.h
public typealias DataConcernType = ODWDataConcernType

// ODWCommonDataContext.h
public typealias CommonDataContext = ODWCommonDataContext

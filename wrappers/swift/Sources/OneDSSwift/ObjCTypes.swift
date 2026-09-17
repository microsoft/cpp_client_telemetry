//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Contains alias for the types declared in the ObjC header files to make them available
/// as part of the swift package module.
/// This lets clients use the Swift package module without importing the Obj-C module explicitly.
/// Important: Due to objc->swift conventions, Type name is removed, so ODWPiiKindGenericData would be accessed as .genericData in swift.

/// Check corresponding header file for the doc of each type.

#if canImport(MATTelemetryObjC)
import MATTelemetryObjC
#else
import ObjCModule
#endif

// ODWEventProperties.h
public typealias EventPriority = ODWEventPriority
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
#if MATSDK_PRIVACYGUARD_AVAILABLE
public typealias DataConcernType = ODWDataConcernType
#endif

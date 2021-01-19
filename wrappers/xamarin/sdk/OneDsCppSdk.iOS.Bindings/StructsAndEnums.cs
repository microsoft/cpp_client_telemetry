//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using ObjCRuntime;

namespace Microsoft.Applications.Events
{
    [Native]
    public enum EventPriority : long
    {
        Unspecified = -1,
        Off = 0,
        Low = 1,
        Normal = 2,
        High = 3,
        Immediate = 4
    }

    [Native]
    public enum PiiKind : long
    {
        None = 0,
        DistinguishedName = 1,
        GenericData = 2,
        IPv4Address = 3,
        IPv6Address = 4,
        MailSubject = 5,
        PhoneNumber = 6,
        QueryString = 7,
        SipAddress = 8,
        SmtpAddress = 9,
        Identity = 10,
        Uri = 11,
        Fqdn = 12,
        IPV4AddressLegacy = 13
    }

    [Flags]
    public enum PrivacyDataType : ulong
    {
        BrowsingHistory = 0x2L,
        DeviceConnectivityAndConfiguration = 0x800L,
        InkingTypingAndSpeechUtterance = 0x20000L,
        ProductAndServicePerformance = 0x1000000L,
        ProductAndServiceUsage = 0x2000000L,
        SoftwareSetupAndInventory = 0x80000000L
    }

    public enum DiagLevel : byte
    {
        RequiredDiagnosticData = 1,
        OptionalDiagnosticData = 2,
        RequiredServiceData = 110,
        RequiredServiceDataForEssentialServices = 120
    }

    [Native]
    public enum TraceLevel : long
    {
        None = 0,
        Error = 1,
        Warning = 2,
        Information = 3,
        Verbose = 4
    }

    [Native]
    public enum SessionState : long
    {
        Started = 0,
        Ended = 1
    }

    [Native]
    public enum TransmissionProfile : long
    {
        RealTime = 0,
        NearRealTime = 1,
        BestEffort = 2
    }

    [Native]
    public enum Status : long
    {
        Efail = -1,
        Success = 0,
        Eperm = 1,
        Ealready = 2,
        Enosys = 3,
        Enotsup = 4
    }

    [Native]
    public enum DataConcernType : long
    {
        None = 0,
        Content = 1,
        DemographicInfoCountryRegion = 2,
        DemographicInfoLanguage = 3,
        Directory = 4,
        ExternalEmailAddress = 5,
        FieldNameImpliesLocation = 6,
        FileNameOrExtension = 7,
        FileSharingUrl = 8,
        InScopeIdentifier = 9,
        InScopeIdentifierActiveUser = 10,
        InternalEmailAddress = 11,
        IpAddress = 12,
        Location = 13,
        MachineName = 14,
        OutOfScopeIdentifier = 15,
        PIDKey = 16,
        Security = 17,
        Url = 18,
        UserAlias = 19,
        UserDomain = 20,
        UserName = 21
    }
}

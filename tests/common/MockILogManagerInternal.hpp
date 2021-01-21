//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "Common.hpp"
#include "api/LogManagerImpl.hpp"
#include "api/ContextFieldsProvider.hpp"
#include "Enums.hpp"
#include "ILogger.hpp"


namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

    class MockILogManagerInternal : public MAT::ILogManagerInternal
    {
    public:
        MockILogManagerInternal();
        ~MockILogManagerInternal();

        MOCK_METHOD0(FlushAndTeardown, void());
        MOCK_METHOD0(Flush, MAT::status_t());
        MOCK_METHOD0(UploadNow, MAT::status_t());
        MOCK_METHOD0(PauseTransmission, MAT::status_t());
        MOCK_METHOD0(ResumeTransmission, MAT::status_t());
        MOCK_METHOD1(SetTransmitProfile, MAT::status_t(MAT::TransmitProfile profile));
        MOCK_METHOD1(SetTransmitProfile, MAT::status_t(const std::string& profile));
        MOCK_METHOD1(LoadTransmitProfiles, MAT::status_t(const std::string& profiles_json));
        MOCK_METHOD0(ResetTransmitProfiles, MAT::status_t());
        MOCK_METHOD0(GetTransmitProfileName, std::string&());
        MOCK_METHOD0(GetSemanticContext, MAT::ISemanticContext & ());
        MOCK_METHOD3(SetContext, MAT::status_t(std::string const &, std::string const &, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, const char*, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, double, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, int64_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, int8_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, int16_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, int32_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, uint8_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, uint16_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, uint32_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, uint64_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, bool, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, MAT::time_ticks_t, MAT::PiiKind));
        MOCK_METHOD3(SetContext, MAT::status_t(const std::string&, MAT::GUID_t, MAT::PiiKind));
        using MAT::ILogManagerInternal::GetLogger;
        MOCK_METHOD4(GetLogger, MAT::ILogger * (std::string const &, MAT::ContextFieldsProvider*, std::string const &, std::string const &));
        MOCK_METHOD1(sendEvent, void(MAT::IncomingEventContextPtr const &));
    };

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing


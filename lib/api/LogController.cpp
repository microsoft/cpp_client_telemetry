// Copyright (c) Microsoft. All rights reserved.
#include "pal/PAL.hpp"
#include "LogController.hpp"
#include "CommonLogManagerInternal.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {

ARIASDK_LOG_INST_COMPONENT_CLASS(LogController, "AriaSDK.LogController", "Aria telemetry client - LogController class");

LogController::LogController()
{
    ARIASDK_LOG_DETAIL("New LogController instance");
}

LogController::~LogController()
{
    ARIASDK_LOG_INFO("destructor");
}

ACTStatus LogController::Flush()
{
    ARIASDK_LOG_ERROR("Flush() is not implemented");
    return CommonLogManagerInternal::Flush();
}

ACTStatus LogController::UploadNow()
{
    return CommonLogManagerInternal::UploadNow();
}

ACTStatus LogController::PauseTransmission()
{
    ARIASDK_LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
    return CommonLogManagerInternal::PauseTransmission();
}

ACTStatus LogController::ResumeTransmission()
{
    ARIASDK_LOG_INFO("Resuming transmission...");
    return CommonLogManagerInternal::ResumeTransmission();
}

/// <summary>Sets the transmit profile by enum</summary>
/// <param name="profile">Profile enum</param>
ACTStatus LogController::SetTransmitProfile(TransmitProfile profile)
{
    return CommonLogManagerInternal::SetTransmitProfile(profile);
}

/// <summary>
/// Select one of several predefined transmission profiles.
/// </summary>
/// <param name="profile"></param>
ACTStatus LogController::SetTransmitProfile(const std::string& profile)
{
    ARIASDK_LOG_INFO("SetTransmitProfile: profile=%s", profile.c_str());
    return CommonLogManagerInternal::SetTransmitProfile(profile);
}

/// <summary>
/// Load transmit profiles from JSON config
/// </summary>
/// <param name="profiles_json">JSON config (see example above)</param>
/// <returns>true on successful profiles load, false if config is invalid</returns>
ACTStatus LogController::LoadTransmitProfiles(const std::string& profiles_json)
{
    ARIASDK_LOG_INFO("LoadTransmitProfiles");
    return CommonLogManagerInternal::LoadTransmitProfiles(profiles_json);
}

/// <summary>
/// Reset transmission profiles to default settings
/// </summary>
ACTStatus LogController::ResetTransmitProfiles()
{
    ARIASDK_LOG_INFO("ResetTransmitProfiles");
    return CommonLogManagerInternal::ResetTransmitProfiles();
}

const std::string& LogController::GetTransmitProfileName()
{
    return CommonLogManagerInternal::GetTransmitProfileName();
};

/// <summary>
/// Add Debug callback
/// </summary>
void LogController::AddEventListener(DebugEventType type, DebugEventListener &listener)
{
    CommonLogManagerInternal::AddEventListener(type, listener);
}

/// <summary>
/// Remove Debug callback
/// </summary>
void LogController::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
{
    CommonLogManagerInternal::RemoveEventListener(type, listener);
}

}}} // namespace Microsoft::Applications::Telemetry

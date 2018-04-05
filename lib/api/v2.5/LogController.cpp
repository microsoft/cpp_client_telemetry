// Copyright (c) Microsoft. All rights reserved.
#include "pal/PAL.hpp"
#include "LogController.hpp"
#include "LogManager.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {

ARIASDK_LOG_INST_COMPONENT_CLASS(LogController, "EventsSDK.LogController", "Events telemetry client - LogController class");

LogController::LogController()
{
    LOG_TRACE("New LogController instance");
}

LogController::~LogController()
{
    LOG_INFO("destructor");
}

EVTStatus LogController::Flush()
{
    LOG_ERROR("Flush() is not implemented");
    return LogManager::Flush();
}

EVTStatus LogController::UploadNow()
{
    return LogManager::UploadNow();
}

EVTStatus LogController::PauseTransmission()
{
    LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
    return LogManager::PauseTransmission();
}

EVTStatus LogController::ResumeTransmission()
{
    LOG_INFO("Resuming transmission...");
    return LogManager::ResumeTransmission();
}

/// <summary>Sets the transmit profile by enum</summary>
/// <param name="profile">Profile enum</param>
EVTStatus LogController::SetTransmitProfile(TransmitProfile profile)
{
    return LogManager::SetTransmitProfile(profile);
}

/// <summary>
/// Select one of several predefined transmission profiles.
/// </summary>
/// <param name="profile"></param>
EVTStatus LogController::SetTransmitProfile(const std::string& profile)
{
    LOG_INFO("SetTransmitProfile: profile=%s", profile.c_str());
    return LogManager::SetTransmitProfile(profile);
}

/// <summary>
/// Load transmit profiles from JSON config
/// </summary>
/// <param name="profiles_json">JSON config (see example above)</param>
/// <returns>true on successful profiles load, false if config is invalid</returns>
EVTStatus LogController::LoadTransmitProfiles(const std::string& profiles_json)
{
    LOG_INFO("LoadTransmitProfiles");
    return LogManager::LoadTransmitProfiles(profiles_json);
}

/// <summary>
/// Reset transmission profiles to default settings
/// </summary>
EVTStatus LogController::ResetTransmitProfiles()
{
    LOG_INFO("ResetTransmitProfiles");
    return LogManager::ResetTransmitProfiles();
}

const std::string& LogController::GetTransmitProfileName()
{
    return LogManager::GetTransmitProfileName();
}

EVTStatus LogController::SetAuthenticationStrictMode(bool value)
{
    return LogManager::GetAuthTokensController()->SetStrictMode(value);
}

/// <summary>
/// Add Debug callback
/// </summary>
void LogController::AddEventListener(DebugEventType type, DebugEventListener &listener)
{
    LogManager::AddEventListener(type, listener);
}

/// <summary>
/// Remove Debug callback
/// </summary>
void LogController::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
{
    LogManager::RemoveEventListener(type, listener);
}

}}} // namespace Microsoft::Applications::Events 

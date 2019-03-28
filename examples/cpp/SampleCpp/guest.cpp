// This is an example that shows how ato utilize SDK "Guest" mode.
// SDK running in Guest mode cannot utilize the LogController methods.

//// This define below illustrates how to enforce the compile-time method availability check
// #define SDK_GUEST_MODE

#include "LogManagerA.hpp"
#include "LogManagerB.hpp"

#include <cassert>

using namespace MAT;

#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"

extern "C" void guestTest()
{

    {
        auto& config = LogManagerA::GetLogConfiguration();
        config["name"] = "ModuleA";
        config["version"] = "1.2.5";
        config["config"] = { { "host", "*" } }; // Any host
    }

    {
        auto& config = LogManagerB::GetLogConfiguration();
        config["name"] = "ModuleB";
        config["version"] = "1.2.5";
        config["config"] = { { "host", "*" } }; // Any host
    }

    auto loggerA = LogManagerA::Initialize(TOKEN);
    auto loggerB = LogManagerB::Initialize(TOKEN);

    loggerA->LogEvent("HelloFromModuleA");
    loggerB->LogEvent("HelloFromModuleB");

    auto controller = LogManagerA::GetController();
    controller->PauseTransmission();

#ifndef SDK_GUEST_MODE
    std::string profile = "someProfile";
    std::string profiles_json = "{}";
    // This example illustrates how to use runtime method permission check
    assert(LogManagerA::UploadNow()          == STATUS_EPERM);
    assert(LogManagerA::Flush()              == STATUS_EPERM);
    assert(LogManagerA::PauseTransmission()  == STATUS_EPERM);
    assert(LogManagerA::ResumeTransmission() == STATUS_EPERM);
    assert(LogManagerA::SetTransmitProfile(TransmitProfile_RealTime) == STATUS_EPERM);
    assert(LogManagerA::SetTransmitProfile(profile) == STATUS_EPERM);
    assert(LogManagerA::LoadTransmitProfiles(profiles_json) == STATUS_EPERM);
    assert(LogManagerA::ResetTransmitProfiles() == STATUS_EPERM);
#endif

}

// Copyright (c) Microsoft. All rights reserved.
#ifndef TRANSMITPROFILESSTUB_HPP
#define TRANSMITPROFILESSTUB_HPP
#include <TransmitProfiles.hpp>

namespace ARIASDK_NS_BEGIN {

    TransmitProfiles::TransmitProfiles() {};

    TransmitProfiles::~TransmitProfiles() {};

    bool TransmitProfiles::load(const std::string&) { return false; };

    bool TransmitProfiles::load(const std::vector<TransmitProfileRules>&) noexcept { return false; };

    void TransmitProfiles::reset() { };

    bool TransmitProfiles::setDefaultProfile(const TransmitProfile) { return false; };

    bool TransmitProfiles::setProfile(const std::string&) { return false; }

    std::string& TransmitProfiles::getProfile() { static std::string def = ""; return def; }

    bool TransmitProfiles::updateStates(NetworkCost, PowerSource) { return true; };

    void TransmitProfiles::getTimers(TimerArray& out)
    {
        out[0] = 1;
        out[1] = 2;
    }

    bool TransmitProfiles::isTimerUpdateRequired() { return false; }

} ARIASDK_NS_END

#endif // TRANSMITPROFILESSTUB_HPP

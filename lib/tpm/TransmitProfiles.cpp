//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "TransmitProfiles.hpp"
#include "pal/PAL.hpp"

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

#include "utils/Utils.hpp"

#include <mutex>
#include <set>

using namespace MAT;
using namespace std;

#ifdef _WIN32
#include <windows.h> // for EXCEPTION_ACCESS_VIOLATION
#include <excpt.h>
#endif

static const char* const defaultRealTimeProfileName = "REAL_TIME";
static const char* const defaultNearRealTimeProfileName = "NEAR_REAL_TIME";
static const char* const defaultBestEffortProfileName = "BEST_EFFORT";

static const set<string, std::greater<string>> defaultProfileNames = {
    string{defaultRealTimeProfileName},
    string{defaultNearRealTimeProfileName},
    string{defaultBestEffortProfileName}
};

static const char* DEFAULT_PROFILE = defaultRealTimeProfileName;

/// <summary>
/// Runtime map of text fields to struct fields and their types.
/// This map greatly helps to simplify the serialization from JSON to binary.
/// </summary>
#ifdef HAVE_MAT_JSONHPP
static std::map<std::string, int > transmitProfileNetCost;
static std::map<std::string, int > transmitProfilePowerState;

static void initTransmitProfileFields()
{
    transmitProfileNetCost["any"] = (NetworkCost_Any);
    transmitProfileNetCost["unknown"] = (NetworkCost_Unknown);
    transmitProfileNetCost["unmetered"] = (NetworkCost_Unmetered);
    transmitProfileNetCost["low"] = (NetworkCost_Unmetered);
    transmitProfileNetCost["metered"] = (NetworkCost_Metered);
    transmitProfileNetCost["high"] = (NetworkCost_Metered);
    transmitProfileNetCost["restricted"] = (NetworkCost_Roaming);
    transmitProfileNetCost["roaming"] = (NetworkCost_Roaming);

    transmitProfilePowerState["any"] = (PowerSource_Any);
    transmitProfilePowerState["unknown"] = (PowerSource_Unknown);
    transmitProfilePowerState["battery"] = (PowerSource_Battery);
    transmitProfilePowerState["charging"] = (PowerSource_Charging);
};
#endif

#define LOCK_PROFILES       std::lock_guard<std::recursive_mutex> lock(profiles_mtx)

namespace MAT_NS_BEGIN {

    static std::recursive_mutex profiles_mtx;
    map<string, TransmitProfileRules>      TransmitProfiles::profiles;
    string      TransmitProfiles::currProfileName = DEFAULT_PROFILE;
    size_t      TransmitProfiles::currRule = 0;
    NetworkCost TransmitProfiles::currNetCost = NetworkCost::NetworkCost_Any;
    PowerSource TransmitProfiles::currPowState = PowerSource::PowerSource_Any;
    bool        TransmitProfiles::isTimerUpdated = true;

    /// <summary>
    /// Get current transmit profile name
    /// </summary>
    /// <returns></returns>
    std::string& TransmitProfiles::getProfile() {
        LOCK_PROFILES;
        return currProfileName;
    };

    /// <summary>
    /// Get current device network and power state
    /// </summary>
    /// <returns></returns>
    void TransmitProfiles::getDeviceState(NetworkCost &netCost, PowerSource &powState) {
        LOCK_PROFILES;
        netCost = currNetCost;
        powState = currPowState;
    };

    /// <summary>
    /// Print transmit profiles to debug log
    /// </summary>
    void TransmitProfiles::dump() {
#ifdef HAVE_MAT_LOGGING
        LOCK_PROFILES;
        for (auto &kv : profiles) {
            auto &profile = kv.second;
            LOG_TRACE("name=%s", profile.name.c_str());
            size_t i = 0;
            for (auto &rule : profile.rules) {
                LOG_TRACE("[%d] netCost=%2d, powState=%2d, timers=[%3d,%3d,%3d]",
                    i, rule.netCost, rule.powerState,
                    rule.timers[0],
                    rule.timers[1],
                    rule.timers[2]);
                i++;
            }
        }
#endif
    }

    /// <summary>
    /// Remove custom profiles. This function is only called from parse and does not require the lock.
    /// </summary>
    void TransmitProfiles::removeCustomProfiles() {
        auto it = profiles.begin();
        while (it != profiles.end())
        {
            if (defaultProfileNames.find((*it).first) != defaultProfileNames.end()) {
                ++it;
                continue;
            }
            it = profiles.erase(it);
        }
    }

    void TransmitProfiles::UpdateProfiles(const std::vector<TransmitProfileRules>& newProfiles) noexcept
    {
        LOCK_PROFILES;
        removeCustomProfiles();
        // Add new profiles
        for (const auto& profile : newProfiles)
        {
            profiles[profile.name] = profile;
        }
        // Check if profile is still valid. If no such profile loaded anymore, then switch to default.
        auto it = profiles.find(currProfileName);
        if (it == profiles.end())
        {
            currProfileName = DEFAULT_PROFILE;
            LOG_TRACE("Switched to profile %s", currProfileName.c_str());
        }

#ifdef  HAVE_MAT_LOGGING
        // Print combined list of profiles: default + custom
        LOG_TRACE("Profiles:");
        size_t i = 0;
        for (const auto& kv : profiles)
        {
            LOG_TRACE("[%d] %s%s", i, kv.first.c_str(),
                        (!kv.first.compare(currProfileName)) ? " [active]" : "");
            i++;
        }
#endif  
        currRule = 0;
        updateStates(currNetCost, currPowState);
    }

	 void TransmitProfiles::EnsureDefaultProfiles() noexcept
	 {
        LOCK_PROFILES;
        if (profiles.size() == 0)
        {
            LOG_TRACE("Loading default profiles...");
            reset();
        }
    }

    /// <summary>
    /// Parse JSON configration describing transmit profiles
    /// </summary>
    /// <param name="profiles_json"></param>
    /// <param name="profiles"></param>
    /// <returns></returns>
#ifdef HAVE_MAT_JSONHPP
    size_t TransmitProfiles::parse(const std::string& profiles_json)
    {
        static const char* attributeName = "name";
        static const char* attributeRules = "rules";
        size_t numProfilesParsed = 0;
        // Temporary storage for the new profiles that we use before we copy to current profiles
        std::vector<TransmitProfileRules> newProfiles;

        using nlohmann::json;
        try
        {
            json temp = json::parse(profiles_json.c_str());

            // Try to parse the JSON string into result variant
            if (temp.is_array())
            {
                size_t numProfiles = temp.size();
                if (numProfiles > MAX_TRANSMIT_PROFILES) {
                    goto parsing_failed;

                }
                LOG_TRACE("got %u profiles", numProfiles);
                for (auto it = temp.begin(); it != temp.end(); ++it)
                {
                    TransmitProfileRules profile;
                    json rulesObj = it.value();
                    if (rulesObj.is_object())
                    {
                        std::string name = rulesObj[attributeName];

                        profile.name = name;
                        json rules = rulesObj[attributeRules];

                        if (rules.is_array())
                        {
                            size_t numRules = rules.size();
                            if (numRules > MAX_TRANSMIT_RULES)
                            {
                                LOG_ERROR("Exceeded max transmit rules %d>%d for profile",
                                    numRules, MAX_TRANSMIT_RULES);
                                goto parsing_failed;
                            }

                            profile.rules.clear();
                            for (auto itRule = rules.begin(); itRule != rules.end(); ++itRule)
                            {
                                if (itRule.value().is_object())
                                {
                                    TransmitProfileRule rule;
                                    auto itnetCost = itRule.value().find("netCost");
                                    if (itRule.value().end() != itnetCost)
                                    {
                                        std::string netCost = itRule.value()["netCost"];
                                        std::map<std::string, int>::const_iterator iter = transmitProfileNetCost.find(netCost);
                                        if (iter != transmitProfileNetCost.end())
                                        {
                                            rule.netCost = static_cast<NetworkCost>(iter->second);
                                        }
                                    }

                                    auto itpowerState = itRule.value().find("powerState");
                                    if (itRule.value().end() != itpowerState)
                                    {
                                        std::string powerState = itRule.value()["powerState"];
                                        std::map<std::string, int>::const_iterator iter = transmitProfilePowerState.find(powerState);
                                        if (iter != transmitProfilePowerState.end())
                                        {
                                            rule.powerState = static_cast<PowerSource>(iter->second);
                                        }
                                    }

                                    auto timers = itRule.value()["timers"];

                                    for (const auto& timer : timers)
                                    {
                                        if (timer.is_number())
                                        {
                                            rule.timers.push_back(timer);
                                        }
                                    }
                                    profile.rules.push_back(rule);
                                }

                            }
                        }
                    }
                    newProfiles.push_back(profile);
                }
            }
        }
        catch (...)
        {
            LOG_ERROR("JSON parsing failed miserably! Please check your config to fix above errors.");
        }

        numProfilesParsed = newProfiles.size();
        UpdateProfiles(newProfiles);
        LOG_INFO("JSON parsing completed successfully [%d]", numProfilesParsed);


        if (numProfilesParsed == 0) {
        parsing_failed:
            LOG_ERROR("JSON parsing failed miserably! Please check your config to fix above errors.");
        }
        return numProfilesParsed;
    }
#else
    size_t TransmitProfiles::parse(const std::string&)
    {
        return size_t { 0 };
    }
#endif // HAVE_MAT_JSONHPP

    /// <summary>
    /// Load customer supplied transmit profiles
    /// </summary>
    /// <param name="profiles_json"></param>
    /// <returns></returns>
    bool TransmitProfiles::load(const std::string& profiles_json) {
        EnsureDefaultProfiles();
        // Check if custom profile is valid
        LOG_TRACE("Loading custom profiles...");
        bool result = (parse(profiles_json) != 0);
        // Dump the current profile to debug log
        dump();
        return result;
    }

    /// <summary>
    /// Load customer supplied transmit profiles
    /// </summary>
    /// <param name="profiles"></param>
    /// <returns></returns>
    bool TransmitProfiles::load(const std::vector<TransmitProfileRules>& profileCandidates) noexcept
    {
        EnsureDefaultProfiles();
        LOG_TRACE("Loading custom profiles...");

        if (profileCandidates.size() > MAX_TRANSMIT_PROFILES)
        {
            LOG_ERROR("Exceeded max transmit profiles %d>%d.", profileCandidates.size(), MAX_TRANSMIT_PROFILES);
            return false;
        }

        for (const auto& profile : profileCandidates)
        {
            const auto ruleCount = profile.rules.size();
            if (ruleCount > MAX_TRANSMIT_RULES)
            {
                LOG_ERROR("Exceeded max transmit rules %d>%d for profile", ruleCount, MAX_TRANSMIT_RULES);
                return false;
            }
            else if (ruleCount == 0)
            {
                LOG_ERROR("Profile must have at least one rule");
                return false;
            }
            for (const auto& rule : profile.rules)
            {
                if (rule.timers.size() != 3)
                {
                    LOG_ERROR("Rule must have three timer values.");
                    return false;
                }
            }
        }

        UpdateProfiles(profileCandidates);

        dump();
        return true;
    }

    /// <summary>
    /// Reset transmit profiles to defaults.
    /// </summary>
    void TransmitProfiles::reset() {
        const TransmitProfileRules realTimeProfile{ std::string{ defaultRealTimeProfileName },
        {
            { NetworkCost::NetworkCost_Roaming, {-1, -1, -1} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {16, 8, 4} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {16, 8, 4} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {12, 6, 3} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {8, 4, 2} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {8, 4, 2} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {4, 2, 1} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {8, 4, 2} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {8, 4, 2} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {4, 2, 1} },
            { {-1, -1, -1} }
        }};

        const TransmitProfileRules nearRealTimeProfile{ std::string{ defaultNearRealTimeProfileName },
        {
            { NetworkCost::NetworkCost_Roaming, {-1, -1, -1} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {-1, 24, 12} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {-1, 24, 12} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {-1, 18, 9} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {24, 12, 6} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {24, 12, 6} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {12, 6, 3} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {24, 12, 6} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {24, 12, 6} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {12, 6, 3} },
            { {-1, -1, -1} }
        }};

        const TransmitProfileRules bestEffortProfile{ std::string{ defaultBestEffortProfileName },
        {
            { NetworkCost::NetworkCost_Roaming, {-1, -1, -1} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {-1, 72, 36} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {-1, 72, 36} },
            { NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {-1, 54, 27} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {72, 36, 18} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {72, 36, 18} },
            { NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {36, 18, 9} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {72, 36, 18} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {72, 36, 18} },
            { NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {36, 18, 9} },
            { {-1, -1, -1} }
        }};

        UpdateProfiles({realTimeProfile, nearRealTimeProfile, bestEffortProfile});
    }

    bool TransmitProfiles::setDefaultProfile(const TransmitProfile profileName)
    {
        std::string selectedProfileName;
        int index = 0;
        std::set<std::string>::iterator it;
        for (it = defaultProfileNames.begin(); it != defaultProfileNames.end(); ++it)
        {
            selectedProfileName = *it;
            if (index == profileName)
            {
                break;
            }
            index++;
        }
        return setProfile(selectedProfileName);
    }


    /// <summary>
    /// Set active profile by name.
    /// 
    /// If profile is found, this function applies the profile and returns true.
    /// 
    /// If profile is not found in configuration (user-error),
    /// then default REAL_TIME profile is applied and function returns false.
    /// 
    /// </summary>
    /// <param name="profileName">Name of a profile to be applied</param>
    /// <returns>true if profile is applied, false otherwise</returns>
    bool TransmitProfiles::setProfile(const std::string& profileName) {
        bool result = false;

        EnsureDefaultProfiles();
        LOCK_PROFILES;
        auto it = profiles.find(profileName);
        if (it != profiles.end()) {
            currProfileName = profileName;
            LOG_INFO("selected profile %s ...", profileName.c_str());
            result = true;
        }
        else {
            LOG_WARN("profile %s not found!", profileName.c_str());
            currProfileName = DEFAULT_PROFILE;
            LOG_WARN("selected profile %s instead", currProfileName.c_str());
        }
        updateStates(currNetCost, currPowState);
        return result;
    }

    /// <summary>
    /// Get the current list of priority timers
    /// </summary>
    /// <returns></returns>
    void TransmitProfiles::getTimers(TimerArray& out) {
        EnsureDefaultProfiles();

        LOCK_PROFILES;
        auto it = profiles.find(currProfileName);
        // When we can't get timers, we won't set isTimerUpdated to false,
        // so we will keep calling getTimers from TransmissionPolicyManager.
        if (it == profiles.end()) {
            out.fill(-1);
            LOG_WARN("No active profile found, disabling all transmission timers.");
            return;
        }
        if (currRule >= it->second.rules.size()) {
            out.fill(-1);
            LOG_ERROR(
                "Profile %s current rule %iz >= profile length %iz",
                currProfileName.c_str(),
                currRule,
                it->second.rules.size()
            );
            return;
        }
        auto const & rule = (it->second).rules[currRule];
        if (rule.timers.empty()) {
            out.fill(-1);
            LOG_ERROR(
                "Profile %s rule %iz has no timers",
                currProfileName.c_str(),
                currRule
            );
            return;
        }
        out[0] = 1000 * rule.timers[0];
        out[1] = out[0];
        if (rule.timers.size() > 2) {
            out[1] = 1000 * rule.timers[2];
        }
        isTimerUpdated = false;
    }

    /// <summary>
    /// 
    /// </summary>
    bool TransmitProfiles::isTimerUpdateRequired()
    {
        LOCK_PROFILES;
        return isTimerUpdated;
    }

    /// <summary>
    /// This function is called only from updateStates
    /// </summary>
    void TransmitProfiles::onTimersUpdated() {
        isTimerUpdated = true;
#ifdef HAVE_MAT_LOGGING
        auto it = profiles.find(currProfileName);
        if (it != profiles.end()) {
            /* Debug routine to print the list of currently selected timers */
            TransmitProfileRule &rule = (it->second).rules[currRule];
            // Print just 3 timers for now because we support only 3
            LOG_INFO("timers=[%3d,%3d,%3d]",
                rule.timers[0],
                rule.timers[1],
                rule.timers[2]);
        }
#endif
    }

    /// <summary>
    /// Select profile rule based on current device state
    /// </summary>
    /// <param name="netCost"></param>
    /// <param name="powState"></param>
    bool TransmitProfiles::updateStates(NetworkCost netCost, PowerSource powState) {
        bool result = false;

        LOCK_PROFILES;
        currNetCost = netCost;
        currPowState = powState;
        auto it = profiles.find(currProfileName);
        if (it != profiles.end()) {
            auto &profile = it->second;
            // Search for a matching rule. If not found, then return the first (the most restrictive) rule in the list.
            currRule = 0;
            for (size_t i = 0; i < profile.rules.size(); i++) {
                const auto &rule = profile.rules[i];
                if ((
                    (rule.netCost == netCost) || (NetworkCost::NetworkCost_Any == netCost) || (NetworkCost::NetworkCost_Any == rule.netCost)) &&
                    ((rule.powerState == powState) || (PowerSource::PowerSource_Any == powState) || (PowerSource::PowerSource_Any == rule.powerState))
                    )
                {
                    currRule = i;
                    result = true;
                    break;
                }
            }
            onTimersUpdated();
        }
        return result;
    }

    TransmitProfiles::TransmitProfiles()
    {
#ifdef HAVE_MAT_JSONHPP
        initTransmitProfileFields();
#endif
    }

    // Make sure we populate transmitProfileFields dynamically before start
    static TransmitProfiles __profiles;

} MAT_NS_END

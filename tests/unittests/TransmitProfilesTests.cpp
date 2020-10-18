//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include <TransmitProfiles.hpp>

using namespace testing;
using namespace MAT;

class TransmitProfilesTests : public TransmitProfiles, public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        TransmitProfiles::profiles.clear();
    }

    virtual void TearDown() override
    {
        TransmitProfiles::profiles.clear();
    }
};

class TransmitProfilesTests_EnsureDefaultProfiles : public TransmitProfilesTests
{
protected:
    virtual void SetUp() override
    {
        TransmitProfilesTests::SetUp();
        TransmitProfiles::EnsureDefaultProfiles();
    }

    const char* const defaultRealTimeProfileName = "REAL_TIME";
    const char* const defaultNearRealTimeProfileName = "NEAR_REAL_TIME";
    const char* const defaultBestEffortProfileName = "BEST_EFFORT";

    void ValidateTransmitProfileRule(const TransmitProfileRule& rule, const std::vector<int>& expectedTimers)
    {
        ValidateTransmitProfileRule(rule, NetworkCost_Any, expectedTimers);
    }

    void ValidateTransmitProfileRule(const TransmitProfileRule& rule, NetworkCost expectedNetCost, const std::vector<int>& expectedTimers)
    {
        ValidateTransmitProfileRule(rule, expectedNetCost, PowerSource_Any, expectedTimers);
    }

    void ValidateTransmitProfileRule(const TransmitProfileRule& rule, NetworkCost expectedNetCost, PowerSource expectedPowerSource, const std::vector<int>& expectedTimers)
    {
        ASSERT_EQ(rule.netCost, expectedNetCost);
        ASSERT_EQ(rule.powerState, expectedPowerSource);
        ASSERT_EQ(rule.timers.size(), expectedTimers.size());
        ASSERT_EQ(rule.timers[0], expectedTimers[0]);
        ASSERT_EQ(rule.timers[1], expectedTimers[1]);
        ASSERT_EQ(rule.timers[2], expectedTimers[2]);
    }
};

TEST_F(TransmitProfilesTests, EnsureDefaultProfiles_NeverCalledBefore_ProfilesSizeIsThree)
{
    ASSERT_EQ(TransmitProfiles::profiles.size(), size_t{0});
    TransmitProfiles::EnsureDefaultProfiles();
    ASSERT_EQ(TransmitProfiles::profiles.size(), size_t{3});
}

TEST_F(TransmitProfilesTests, EnsureDefaultProfiles_CalledTwice_ProfilesSizeIsThree)
{
    ASSERT_EQ(TransmitProfiles::profiles.size(), size_t{0});
    TransmitProfiles::EnsureDefaultProfiles();
    TransmitProfiles::EnsureDefaultProfiles();
    ASSERT_EQ(TransmitProfiles::profiles.size(), size_t{3});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeProfileEntryMatchesKey)
{
    ASSERT_EQ(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].name, defaultRealTimeProfileName);
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeProfileHasElevenRules)
{
    const auto& profile = TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}];
    ASSERT_EQ(profile.rules.size(), size_t{11});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleZeroIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[0], NetworkCost::NetworkCost_Roaming, {-1, -1, -1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleOneIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[1], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {16, 8, 4});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleTwoIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[2], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {16, 8, 4});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleThreeIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[3], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {12, 6, 3});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleFourIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[4], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {8, 4, 2});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleFiveIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[5], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {8, 4, 2});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleSixIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[6], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {4, 2, 1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleSevenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[7], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {8, 4, 2});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleEightIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[8], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {8, 4, 2});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleNineIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[9], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {4, 2, 1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_RealTimeRuleTenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultRealTimeProfileName}].rules[10], {-1, -1, -1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeProfileEntryMatchesKey)
{
    ASSERT_EQ(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].name, defaultNearRealTimeProfileName);
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeProfileHasElevenRules)
{
    const auto& profile = TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}];
    ASSERT_EQ(profile.rules.size(), size_t{11});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleZeroIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[0], NetworkCost::NetworkCost_Roaming, {-1, -1, -1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleOneIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[1], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {-1, 24, 12});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleTwoIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[2], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {-1, 24, 12});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleThreeIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[3], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {-1, 18, 9});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleFourIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[4], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {24, 12, 6});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleFiveIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[5], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {24, 12, 6});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleSixIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[6], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {12, 6, 3});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleSevenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[7], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {24, 12, 6});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleEightIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[8], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {24, 12, 6});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleNineIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[9], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {12, 6, 3});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_NearRealTimeRuleTenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultNearRealTimeProfileName}].rules[10], {-1, -1, -1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortProfileEntryMatchesKey)
{
    ASSERT_EQ(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].name, defaultBestEffortProfileName);
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortProfileHasElevenRules)
{
    const auto& profile = TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}];
    ASSERT_EQ(profile.rules.size(), size_t{11});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleZeroIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[0], NetworkCost::NetworkCost_Roaming, {-1, -1, -1});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleOneIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[1], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Unknown, {-1, 72, 36});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleTwoIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[2], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Battery, {-1, 72, 36});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleThreeIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[3], NetworkCost::NetworkCost_Metered, PowerSource::PowerSource_Charging, {-1, 54, 27});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleFourIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[4], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Unknown, {72, 36, 18});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleFiveIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[5], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {72, 36, 18});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleSixIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[6], NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging, {36, 18, 9});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleSevenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[7], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Unknown, {72, 36, 18});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleEightIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[8], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Battery, {72, 36, 18});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleNineIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[9], NetworkCost::NetworkCost_Unknown, PowerSource::PowerSource_Charging, {36, 18, 9});
}

TEST_F(TransmitProfilesTests_EnsureDefaultProfiles, Pinning_BestEffortRuleTenIsCorrect)
{
    ValidateTransmitProfileRule(TransmitProfiles::profiles[std::string{defaultBestEffortProfileName}].rules[10], {-1, -1, -1});
}

TEST_F(TransmitProfilesTests, UpdateProfiles_ProfileAlreadyAddedAndNotInNewProfiles_PreviousCustomProfilesRemoved)
{
    const std::string previousRuleName = "previousRule";
    TransmitProfileRules previousRule{previousRuleName, {}};
    TransmitProfiles::profiles[previousRuleName] = previousRule;
    TransmitProfiles::UpdateProfiles(std::vector<TransmitProfileRules>{});
    ASSERT_TRUE(TransmitProfiles::profiles.find(previousRuleName) == TransmitProfiles::profiles.end());
}

TEST_F(TransmitProfilesTests, UpdateProfiles_NewRule_NewRuleAddedToProfiles)
{
    const std::string newRuleName = "newRule";
    TransmitProfileRules newRule{newRuleName, {}};
    TransmitProfiles::UpdateProfiles(std::vector<TransmitProfileRules>{newRule});
    ASSERT_TRUE(TransmitProfiles::profiles.find(newRuleName) != TransmitProfiles::profiles.end());
}

TEST_F(TransmitProfilesTests, UpdateProfiles_OldCurrentProfileDoesntExistInNewRules_SetCurrentProfileToDefault)
{
    TransmitProfiles::currProfileName = std::string{"newRule"};
    TransmitProfiles::UpdateProfiles(std::vector<TransmitProfileRules>{});
    ASSERT_EQ(TransmitProfiles::currProfileName, std::string{"REAL_TIME"});
}

TEST_F(TransmitProfilesTests, UpdateProfiles_OldCurrentProfileExistsInNewRules_CurrentProfileNotChanged)
{
    const std::string newRuleName = "newRule";
    TransmitProfiles::currProfileName = newRuleName;
    TransmitProfileRule rule;
    rule.timers = {1, 2, 3};
    TransmitProfileRules newRule{newRuleName, {rule}};
    TransmitProfiles::UpdateProfiles(std::vector<TransmitProfileRules>{newRule});
    ASSERT_EQ(TransmitProfiles::currProfileName, newRuleName);
}

TEST_F(TransmitProfilesTests, load_VectorIsLargerThanMaxTransmitProfiles_ReturnsFalseAndSizeThree)
{
    std::vector<TransmitProfileRules> newProfiles{MAX_TRANSMIT_PROFILES + 1};
    ASSERT_FALSE(TransmitProfiles::load(newProfiles));
    ASSERT_EQ(TransmitProfiles::profiles.size(), 3);
}

TEST_F(TransmitProfilesTests, load_OneProfileTooManyRules_ReturnsFalseAndSizeThree)
{
    TransmitProfileRules profile;
    profile.rules = std::vector<TransmitProfileRule>{MAX_TRANSMIT_RULES + 1};
    std::vector<TransmitProfileRules> newProfiles{profile};
    ASSERT_FALSE(TransmitProfiles::load(newProfiles));
    ASSERT_EQ(TransmitProfiles::profiles.size(), 3);
}

TEST_F(TransmitProfilesTests, load_OneProfileNoRules_ReturnsFalseAndSizeThree)
{
    TransmitProfileRules profile{"testProfile", { }};
    std::vector<TransmitProfileRules> newProfiles{profile};
    ASSERT_FALSE(TransmitProfiles::load(newProfiles));
    ASSERT_EQ(TransmitProfiles::profiles.size(), 3);
}

TEST_F(TransmitProfilesTests, load_OneProfileOneRuleNoTimers_ReturnsFalseAndSizeThree)
{
    TransmitProfileRule rule;
    TransmitProfileRules profile{"testProfile", { rule }};
    std::vector<TransmitProfileRules> newProfiles{profile};
    ASSERT_FALSE(TransmitProfiles::load(newProfiles));
    ASSERT_EQ(TransmitProfiles::profiles.size(), 3);
}

TEST_F(TransmitProfilesTests, load_OneRule_ReturnsTrueAndSizeFour)
{
    TransmitProfileRule rule;
    rule.timers = std::vector<int>{1, 2, 3};
    TransmitProfileRules profile{"testProfile", { rule }};
    std::vector<TransmitProfileRules> newProfiles{profile};
    ASSERT_TRUE(TransmitProfiles::load(newProfiles));
    ASSERT_EQ(TransmitProfiles::profiles.size(), 4);
}

#ifdef HAVE_MAT_JSONHPP
TEST_F(TransmitProfilesTests, load_Json_EmptyJsonArray_FailsToParse)
{
    const std::string badRule = R"([])";
    ASSERT_FALSE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_JsonArrayContainsOneEmtpyObject_FailsToParse)
{
    const std::string badRule = R"([{ }])";
    ASSERT_FALSE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_OneValidAndOneInvalidRule_ReturnsTrue)
{
    const std::string badRule =
R"([{
         "name": "GoodRule",
         "rules": [
             { "netCost": "restricted", "timers": [ -1, -1, -1 ] }
         ]
},
{
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}

/*
   The following tests probably should not pass.
   But they do.
   Right now these are useful as pinning tests.
*/

TEST_F(TransmitProfilesTests, load_Json_ProfileWithInvalidTimers_ReturnsTrue)
{
    const std::string badRule =
        R"([{
         "name": "EmptyTimersArrayRule",
         "rules": [
             { "timers": [ "a", "b", "c" ] }
         ]
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_ProfileWithEmptyTimerArray_ReturnsTrue)
{
    const std::string badRule = 
R"([{
         "name": "EmptyTimersArrayRule",
         "rules": [
             { "timers": [ ] }
         ]
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_RuleWithoutTimers_ReturnsTrue)
{
    const std::string badRule =
R"([{
         "name": "EmptyRule",
         "rules": [
             { }
         ]
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_EmptyRulesArray_ReturnsTrue)
{
    const std::string badRule =
R"([{
         "name": "EmptyRule",
         "rules": [
         ]
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}

TEST_F(TransmitProfilesTests, load_Json_ProfileWithoutRules_ReturnsTrue)
{
    const std::string badRule =
R"([{
         "name": "EmptyRule"
}])";

    ASSERT_TRUE(TransmitProfiles::load(badRule));
}
#else
TEST_F(TransmitProfilesTests, load_Json_JsonNotEnabled_ReturnsFalse)
{
    const std::string rule = R"([])";
    ASSERT_FALSE(TransmitProfiles::load(rule));
}

TEST_F(TransmitProfilesTests, parse_GoodJsonJsonNotEnabled_ReturnsZero)
{
    const std::string rule = R"([{
         "name": "GoodRule",
         "rules": [
             { "netCost": "restricted", "timers": [ -1, -1, -1 ] }
         ]
}])";
    ;
    ASSERT_EQ(TransmitProfiles::parse(rule), size_t { 0 });
}
#endif  // HAVE_MAT_JSONHPP

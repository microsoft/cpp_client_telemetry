// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include <TransmitProfiles.hpp>

using namespace testing;
using namespace MAT;

class TransmitProfilesTests : public TransmitProfiles, public ::testing::Test
{
    virtual void SetUp() override
    {
        TransmitProfiles::profiles.clear();
    }

    virtual void TearDown() override
    {
        TransmitProfiles::profiles.clear();
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

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "modules/filter/LevelCheckingEventFilter.hpp"

using namespace testing;
using namespace MAT;

class LevelCheckingEventFilterTests : public  ::testing::Test
{
public:
    AllowedLevelsCollection AllowedLevels;
    LevelCheckingEventFilter Filter{ AllowedLevels };
};

TEST_F(LevelCheckingEventFilterTests, Constructor_Default_AllowedLevelsSizeZero)
{
    EXPECT_EQ(size_t { 0 }, AllowedLevels.GetSize());
}

TEST_F(LevelCheckingEventFilterTests, GetName_ReturnsExpectedName)
{
    EXPECT_STREQ("Microsoft::Applications::Events::Internal::LevelCheckingEventFilter", Filter.GetName());
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_NoLevelSet_ReturnsFalse)
{
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LevelCheckingEventFilterTests, UpdateAllowedLevels_EmptySet_SetsAllowedLevelsToSize0)
{
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{});
    EXPECT_EQ(size_t { 0 }, AllowedLevels.GetSize());
}

TEST_F(LevelCheckingEventFilterTests, UpdateAllowedLevels_SizeTwo_SetsAllowedLevelsToSize2)
{
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_EQ(size_t { 2 }, AllowedLevels.GetSize());
}

TEST_F(LevelCheckingEventFilterTests, UpdateAllowedLevels_SizeTwo_AllowedValuesAreSetProperly)
{
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_EQ(42, AllowedLevels[0]);
    EXPECT_EQ(97, AllowedLevels[1]);
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_DefaultAllowedLevelsLevelSetToRequired_ReturnsTrue)
{
    EventProperties properties;
    properties.SetLevel(DIAG_LEVEL_REQUIRED);
    AllowedLevels.UpdateAllowedLevels({ DIAG_LEVEL_REQUIRED });
    EXPECT_TRUE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_DefaultAllowedLevelsLevelSetToNotRequired_ReturnsFalse)
{
    EventProperties properties;
    properties.SetLevel(42);
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_AllowedLevelIs42LevelSetToRequired_ReturnsFalse)
{
    EventProperties properties;
    properties.SetLevel(DIAG_LEVEL_REQUIRED);
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42});
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_AllowedLevelIs42And97LevelSetToRequired_ReturnsFalse)
{
    EventProperties properties;
    properties.SetLevel(DIAG_LEVEL_REQUIRED);
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_AllowedLevelIs42And97LevelSetTo42_ReturnsTrue)
{
    EventProperties properties;
    properties.SetLevel(42);
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_TRUE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_AllowedLevelIs42And97LevelSetTo97_ReturnsTrue)
{
    EventProperties properties;
    properties.SetLevel(97);
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_TRUE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_DisallowedBeforeUpdateAllowedAfter_ReturnsFalseThenTrue)
{
    EventProperties properties;
    properties.SetLevel(97);
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(properties));
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_TRUE(Filter.CanEventPropertiesBeSent(properties));
}

TEST_F(LevelCheckingEventFilterTests, CanEventPropertiesBeSent_AllowedBeforeUpdateDisallowedAfter_ReturnsTrueThenFalse)
{
    EventProperties properties;
    properties.SetLevel(97);
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42, 97});
    EXPECT_TRUE(Filter.CanEventPropertiesBeSent(properties));
    Filter.UpdateAllowedLevels(std::vector<uint8_t>{42});
    EXPECT_FALSE(Filter.CanEventPropertiesBeSent(properties));
}

//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "common/MockILogManagerInternal.hpp"
#include "modules/filter/CompliantByDefaultEventFilterModule.hpp"

using namespace MAT;
using namespace testing;


class CompliantByDefaultEventFilterModuleTests : public CompliantByDefaultEventFilterModule, public ::testing::Test
{
public:
    CompliantByDefaultEventFilterModuleTests() noexcept
        : m_logManager(m_logConfiguration, static_cast<bool>(nullptr)) { }
    ILogConfiguration m_logConfiguration;
    LogManagerImpl m_logManager;

    using CompliantByDefaultEventFilterModule::m_parent;
    using CompliantByDefaultEventFilterModule::m_allowedLevels;

    virtual void SetUp() override
    {
        Initialize(&m_logManager);
    }
};

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_ParentValid_SetsParentMember)
{
    ASSERT_EQ(&m_logManager, m_parent);
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_AllowedLevelsSizeOne)
{
    ASSERT_EQ(m_allowedLevels.GetSize(), size_t { 1 });
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_AllowedLevelValueIsRequired)
{
    ASSERT_TRUE(m_allowedLevels.IsLevelInCollection(DIAG_LEVEL_REQUIRED));
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_HooksIsNotNullptr)
{
    ASSERT_TRUE(m_hooks != nullptr);
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_HooksSizeSet)
{
#ifdef HAVE_MAT_DEFAULT_FILTER
    constexpr const size_t expected = 2;  // This, and the default filter initialized by LogManager.
#else
    constexpr const size_t expected = 1;  // Just this hook.
#endif // HAVE_MAT_DEFAULT_FILTER

    ASSERT_EQ(m_hooks->GetSize(), expected);
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Initialize_ThisPointerIsInHooksCollection)
{
    ASSERT_TRUE(m_hooks->IsModuleInCollection(this));
}

TEST_F(CompliantByDefaultEventFilterModuleTests, UpdateAllowedLevels_UpdatesSetOfAllowedLevels)
{
    UpdateAllowedLevels({ DIAG_LEVEL_OPTIONAL });
    ASSERT_TRUE(m_allowedLevels.IsLevelInCollection(DIAG_LEVEL_OPTIONAL));
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Teardown_HooksIsNullptr)
{
    Teardown();
    ASSERT_EQ(m_hooks, nullptr);
}

TEST_F(CompliantByDefaultEventFilterModuleTests, Teardown_ParentIsNullptr)
{
    Teardown();
    ASSERT_EQ(m_parent, nullptr);
}

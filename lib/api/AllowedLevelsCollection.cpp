//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "AllowedLevelsCollection.hpp"

namespace MAT_NS_BEGIN
{
    AllowedLevelsCollection::AllowedLevelsCollection(std::initializer_list<uint8_t>&& allowedLevels) noexcept
        : m_allowedLevels(allowedLevels)
    { }

    bool AllowedLevelsCollection::IsLevelInCollection(uint8_t level) const noexcept
    {
        std::lock_guard<std::mutex> lock{ m_allowedLevelsLock };
        return std::find(m_allowedLevels.cbegin(), m_allowedLevels.cend(), level) != m_allowedLevels.cend();
    }

    std::size_t AllowedLevelsCollection::GetSize() const noexcept
    {
        std::lock_guard<std::mutex> lock{ m_allowedLevelsLock };
        return m_allowedLevels.size();
    }

    uint8_t AllowedLevelsCollection::operator[](int index) const noexcept
    {
        std::lock_guard<std::mutex> lock{ m_allowedLevelsLock };
        return m_allowedLevels[index];
    }

    void AllowedLevelsCollection::UpdateAllowedLevels(const std::vector<uint8_t>& levels) noexcept
    {
        std::lock_guard<std::mutex> lock{ m_allowedLevelsLock };
        m_allowedLevels = levels;
    }

} MAT_NS_END

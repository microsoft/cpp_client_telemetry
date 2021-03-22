//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ALLOWEDLEVELSCOLLECTION_HPP
#define ALLOWEDLEVELSCOLLECTION_HPP

#include <mutex>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
{

    class AllowedLevelsCollection
    {
    public:
        AllowedLevelsCollection() noexcept = default;
        AllowedLevelsCollection(std::initializer_list<uint8_t>&& allowedLevels) noexcept;

        bool IsLevelInCollection(uint8_t level) const noexcept;
        std::size_t GetSize() const noexcept;
        uint8_t operator[](int index) const noexcept;

        void UpdateAllowedLevels(const std::vector<uint8_t>& levels) noexcept;

    private:
        mutable std::mutex m_allowedLevelsLock;
        std::vector<uint8_t> m_allowedLevels;
    };

} MAT_NS_END

#endif // ALLOWEDLEVELSCOLLECTION_HPP


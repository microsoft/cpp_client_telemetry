#pragma once
#include "mat/config.h"

#ifndef ONDISABLENOTIFICATIONCOLLECTION_HPP
#define ONDISABLENOTIFICATIONCOLLECTION_HPP

#include "public/Version.hpp"
#include "public/ctmacros.hpp"

#include <mutex>
#include <vector>

namespace ARIASDK_NS_BEGIN {

    class OnDisableNotificationCollection
    {
    public:
        void AddOnDisableNotification(const std::function<void()>& callback);
        void TriggerCallbacks() noexcept;

    private:
        std::mutex m_callbacksGuard;
        std::vector<std::function<void()>> m_callbacksCollection;
    };

} ARIASDK_NS_END

#endif
#include "OnDisableNotificationCollection.hpp"

namespace ARIASDK_NS_BEGIN
{
void OnDisableNotificationCollection::AddOnDisableNotification(const std::function<void()>& callback)
{
    if (!callback)
        throw std::invalid_argument("callback");

    std::lock_guard<std::mutex> lock(m_callbacksGuard);
    m_callbacksCollection.push_back(callback);
}

void OnDisableNotificationCollection::TriggerCallbacks() noexcept
{
    std::lock_guard<std::mutex> lock(m_callbacksGuard);
    for (const auto& callback : m_callbacksCollection)
    {
        callback();
    }
}

} ARIASDK_NS_END

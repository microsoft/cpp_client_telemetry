#pragma once

#include <algorithm>
#include <mutex>
#include <vector>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    template <class TObserverType, class TNotifyDataType>
    class ConcurrentObservable
    {
    private:
        std::vector<TObserverType> m_observers;
        std::mutex m_mutex;
        bool m_enabled;

        // This only gets called from the destructor, but it's in a separate method to ensure that the
        // lock is completely and cleanly released before it also gets destructed
        inline void Shutdown()
        {
            std::lock_guard<std::mutex> lockguard(m_mutex);
            m_observers.clear();
            m_enabled = false;
        }

        virtual void NotifyOneObserver(TObserverType target, TNotifyDataType param) = 0;

    public:
        ConcurrentObservable()
        {
            m_enabled = true;
        }

        virtual ~ConcurrentObservable()
        {
            Shutdown();
        }

        void Register(TObserverType observer)
        {
            if (observer == nullptr)
                return;

            std::lock_guard<std::mutex> lockguard(m_mutex);

            if (!m_enabled)
                return;

            typename std::vector<TObserverType>::iterator head = m_observers.begin();

            m_observers.push_back(observer);
        }

        void Unregister(TObserverType observer)
        {
            if (observer == nullptr)
                return;

            std::lock_guard<std::mutex> lockguard(m_mutex);

            if (!m_enabled)
                return;

            m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
        }

        void NotifyObservers(TNotifyDataType NotificationData)
        {
            std::lock_guard<std::mutex> lockguard(m_mutex);

            if (!m_enabled)
                return;

            for (auto observer : m_observers)
            {
                NotifyOneObserver(observer, NotificationData);
            }
        }
    };

    template <class SourceType>
    struct ChangeDataTemplate
    {
        ChangeDataTemplate(SourceType & sourceType, const GUID_t & ariaTenantId)
            : m_ariaTenantId(ariaTenantId), m_source(sourceType)
        {
        }

        SourceType & m_source;
        const GUID_t & m_ariaTenantId;
    };

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane

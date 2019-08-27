#include "pal/WorkerThread.hpp"
#include "pal/PAL.hpp"

#if defined(MATSDK_PAL_CPP11) || defined(MATSDK_PAL_WIN32)

namespace PAL_NS_BEGIN {

    class WorkerThreadShutdownItem : public WorkerThreadItem
    {
    public:
        WorkerThreadShutdownItem()
        {
            type = MAT::WorkerThreadItem::Shutdown;
        }
    };

    class WorkerThread : public IWorkerThread
    {
    protected:
        std::thread                            m_hThread;

        // TODO: [MG] - investigate all the cases why we need recursive here
        std::recursive_mutex                   m_lock;

        std::list<MAT::WorkerThreadItemPtr> m_queue;
        std::list<MAT::WorkerThreadItemPtr> m_timerQueue;
        Event                          m_event;
        MAT::WorkerThreadItemPtr            m_itemInProgress;
        int count = 0;
    
    public:

        WorkerThread()
        {
            m_hThread = std::thread(WorkerThread::threadFunc, static_cast<void*>(this));
            LOG_INFO("Started new thread %u", m_hThread.get_id());
        }

        ~WorkerThread()
        {
            join();
        }

        void join() override
        {
            auto item = new WorkerThreadShutdownItem();
            queue(item);
            std::thread::id this_id = std::this_thread::get_id();
            try {
                if (m_hThread.joinable() && (m_hThread.get_id() != this_id))
                    m_hThread.join();
                else
                    m_hThread.detach();
            }
            catch (...) {};

            // TODO: [MG] - investigate how often that happens.
            // Side-effect is that we have a queued work item discarded on shutdown.
            if (!m_queue.empty())
            {
                LOG_WARN("m_queue is not empty!");
            }
            if (!m_timerQueue.empty())
            {
                LOG_WARN("m_timerQueue is not empty!");
            }
        }

        void queue(MAT::WorkerThreadItemPtr item) override
        {
            // TODO: [MG] - show item type
            LOG_INFO("queue item=%p", &item);
            LOCKGUARD(m_lock);
            if (item->type == MAT::WorkerThreadItem::TimedCall) {
                auto it = m_timerQueue.begin();
                while (it != m_timerQueue.end() && (*it)->targetTime < item->targetTime) {
                    ++it;
                }
                m_timerQueue.insert(it, item);
            }
            else {
                m_queue.push_back(item);
            }
            count++;
            m_event.post();
        }

        bool cancel(MAT::WorkerThreadItemPtr item) override
        {
            if ((m_itemInProgress == item)||(item==nullptr))
            {
                return false;
            }

            {
                LOCKGUARD(m_lock);
                auto it = std::find(m_timerQueue.begin(), m_timerQueue.end(), item);
                if (it != m_timerQueue.end()) {
                    // Still in the queue
                    m_timerQueue.erase(it);
                    delete item;
                    return true;
                }
            }
#if 0
            for (;;) {
                {
                    LOCKGUARD(m_lock);
                    if (item->type == MAT::WorkerThreadItem::Done) {
                        return;
                    }
                }
                Sleep(10);
            }
#endif
            return false;
        }

    protected:
        static void threadFunc(void* lpThreadParameter)
        {
            uint64_t wakeupCount = 0;

            WorkerThread* self = reinterpret_cast<WorkerThread*>(lpThreadParameter);
            LOG_INFO("Running thread %u", std::this_thread::get_id());

            std::unique_ptr<MAT::WorkerThreadItem> item = nullptr;
            for (;;) {
                wakeupCount++;
                unsigned nextTimerInMs = UINT_MAX;
                {
                    LOCKGUARD(self->m_lock);

                    int64_t now = getMonotonicTimeMs();
                    if (!self->m_timerQueue.empty() && self->m_timerQueue.front()->targetTime <= now) {
                        item = std::unique_ptr<MAT::WorkerThreadItem>(self->m_timerQueue.front());
                        self->m_timerQueue.pop_front();
                    }
                    if (!self->m_timerQueue.empty()) {
                        nextTimerInMs = static_cast<unsigned>(self->m_timerQueue.front()->targetTime - now);
                    }

                    if (!self->m_queue.empty() && !item) {
                        item = std::unique_ptr<MAT::WorkerThreadItem>(self->m_queue.front());
                        self->m_queue.pop_front();
                    }
                }

                if (!item) {
                    if (!self->m_event.Reset())
                        self->m_event.wait(nextTimerInMs);
                    continue;
                }

                if (item->type == MAT::WorkerThreadItem::Shutdown) {
                    item.reset();
                    break;
                }
                
                LOG_TRACE("%10llu Execute item=%p type=%s\n", wakeupCount, item.get(), item.get()->typeName.c_str() );
                self->m_itemInProgress = item.get();
                (*item)();
                self->m_itemInProgress = nullptr;

                if (item.get()) {
                    item->type = MAT::WorkerThreadItem::Done;
                    item.reset();
                }
            }
        }
    };

    namespace WorkerThreadFactory {
        MAT::IWorkerThread* Create()
        {
            return new WorkerThread();
        }
    }

} PAL_NS_END

#endif

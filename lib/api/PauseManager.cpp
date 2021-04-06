#include "PauseManager.hpp"

namespace MAT_NS_BEGIN
{
    std::mutex PauseManager::s_pause_manager_mutex;
    std::atomic<uint64_t> PauseManager::s_pause_manager_state(0);
    std::condition_variable PauseManager::s_pause_manager_cv;

    void PauseManager::PauseActivity() noexcept
    {
        auto state = s_pause_manager_state.load();
        while ((state & 1) == 0 && !s_pause_manager_state.compare_exchange_weak(state, state + 1));

        if (state < 2) {
            s_pause_manager_cv.notify_all();
        }
    }

    void PauseManager::ResumeActivity() noexcept
    {
        auto state = s_pause_manager_state.load();
        while ((state & 1) && !s_pause_manager_state.compare_exchange_weak(state, state - 1));
        s_pause_manager_cv.notify_all();
    }

    void PauseManager::QuiesceWait() noexcept
    {
        std::unique_lock<std::mutex> lock(s_pause_manager_mutex);
        s_pause_manager_cv.wait(lock, [] () {
            auto state = s_pause_manager_state.load();
            return state < 2 || !(state & 1);
        });
    }

    void PauseManager::QuiesceWaitFor(std::chrono::milliseconds timeout) noexcept
    {
        std::unique_lock<std::mutex> lock(s_pause_manager_mutex);
        s_pause_manager_cv.wait_for(lock, timeout, [] () {
            auto state = s_pause_manager_state.load();
            return state < 2 || !(state & 1);
        });
    }

    bool PauseManager::TryLock() noexcept
    {
        auto state = PauseManager::s_pause_manager_state.load();
        do {
            if (state & 1) {
                break;
            }
        } while(!PauseManager::s_pause_manager_state.compare_exchange_weak(state, state + 2));
        return (state & 1) == 0;
    }

    void PauseManager::ReleaseLock() noexcept
    {
        auto state = s_pause_manager_state.load();
        do {
            if (state < 2) {
                break;
            }
        } while (!s_pause_manager_state.compare_exchange_weak(state, state - 2));
        if (state < 2) {
            s_pause_manager_cv.notify_all();
        }
    }

    PauseManager::Lock::Lock()
    {
        acquired_lock = PauseManager::TryLock();
    }

    PauseManager::Lock::~Lock()
    {
        if (acquired_lock) {
            PauseManager::ReleaseLock();
        }
    }

    bool PauseManager::Lock::isPaused() const noexcept
    {
        return !acquired_lock;
    }
}
MAT_NS_END

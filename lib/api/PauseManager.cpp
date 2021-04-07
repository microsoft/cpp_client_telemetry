#include "PauseManager.hpp"

namespace MAT_NS_BEGIN
{
    std::mutex PauseManager::s_pause_manager_mutex;
    std::atomic<uint64_t> PauseManager::s_pause_manager_state(0);
    std::condition_variable PauseManager::s_pause_manager_cv;

    // Move from active to pausing/paused state: prevent new activities from starting.
    void PauseManager::PauseActivity() noexcept
    {
        auto state = s_pause_manager_state.load();
        // if we are running (state & 1 is zero), set the pausing/paused bit
        while ((state & 1) == 0 && !s_pause_manager_state.compare_exchange_weak(state, state + 1));

        // if we set the pausing/paused bit and we have no outstanding activity (state < 2),
        // wake up listeners.
        if (state < 2) {
            s_pause_manager_cv.notify_all();
        }
    }

    // Move from pausing/paused to active: new activities may start
    void PauseManager::ResumeActivity() noexcept
    {
        auto state = s_pause_manager_state.load();
        // if we are pausing/paused (state & 1), clear the pausing/paused bit in state.
        while ((state & 1) && !s_pause_manager_state.compare_exchange_weak(state, state - 1));
        // notify any listeners: we are active, so any thread waiting for quiesce should
        // wake up.
        s_pause_manager_cv.notify_all();
    }

    // Wait indefinitely for either the paused (no remaining activity) or running state.
    // If another thead might call ResumeActivity(), the calling thread can wake up
    // on the transition back to the running state, and may need to coordinate
    // with that ResumeActivity().
    void PauseManager::QuiesceWait() noexcept
    {
        std::unique_lock<std::mutex> lock(s_pause_manager_mutex);
        s_pause_manager_cv.wait(lock, [] () -> bool {
            auto state = s_pause_manager_state.load();
            // we wait until either we quiesce (state < 2) or
            // we are active (!state & 1).
            return state < 2 || !(state & 1);
        });
    }

    // Wait for quiesce, but return unconditionally after the supplied
    // timeout duration if quiesce does not happen earlier.
    void PauseManager::QuiesceWaitFor(std::chrono::milliseconds timeout) noexcept
    {
        std::unique_lock<std::mutex> lock(s_pause_manager_mutex);
        s_pause_manager_cv.wait_for(lock, timeout, [] () -> bool {
            auto state = s_pause_manager_state.load();
            return state < 2 || !(state & 1);
        });
    }

    // PauseManager::Lock uses TryLock, and client code should
    // use an instance (typically on the stack) of PauseManager::Lock
    // rather than calling TryLock and ReleaseLock explicitly.
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

    // Only call ReleaseLock if TryLock returned true. Calling ReleaseLock when
    // TryLock returned false or calling ReleaseLock without a corresponding
    // TryLock will set an invalid active count in our state variable.
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

    // On construction, try to acquire a lock. Record whether we received one.
    PauseManager::Lock::Lock()
    {
        acquired_lock = PauseManager::TryLock();
    }

    // On destruction, release a lock if we received one in the constructor.
    PauseManager::Lock::~Lock()
    {
        if (acquired_lock) {
            PauseManager::ReleaseLock();
        }
    }

    // We are paused if we did not receive a lock from the PauseManager.
    bool PauseManager::Lock::isPaused() const noexcept
    {
        return !acquired_lock;
    }
}
MAT_NS_END

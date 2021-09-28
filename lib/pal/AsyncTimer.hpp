//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ASYNC_TIMER_HPP
#define ASYNC_TIMER_HPP

#pragma once

#include <iostream>
#include <mutex>
#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <condition_variable>

namespace common {

    class AsyncTimer {

    protected:
        std::mutex           mtx_cancel;
        std::condition_variable cv;

        std::mutex           mtx_done;
        std::recursive_mutex mtx_params;

        bool                isRunning;
        long                delay;
        long                interval;

        std::chrono::time_point<std::chrono::system_clock> startTime;  // when wait starts
        std::chrono::time_point<std::chrono::system_clock> endTime;    // when wait is expected to end

        // Thread that executes the timer
        std::thread         timer_thread;
        std::atomic<bool>   timer_thread_done;

        // Legacy API parameters
        void               *callback;
        void               *data;

    public:

        /**
         * Construct timer object pointing to Function with Arguments.
         * Start async right away.
         */
        template<class Function, typename... Args>
        AsyncTimer(long delay, long interval, Function&& func, Args ... args) :
            delay(delay), interval(interval), isRunning(false), callback(NULL), data(NULL)
        {
            timer_thread_done.store(false);
            // Capture this, func and args and pass to run(...) method
            timer_thread = std::thread([this, func, args...]() {
                run(func, args...); });
            timer_thread.detach();
        };

        /**
        * Construct timer object pointing to callback with data.
        * Do not start - wait for start.
        */
        AsyncTimer() : delay(0), interval(0), isRunning(false), callback(NULL), data(NULL) {
            timer_thread_done.store(true);
        }

        virtual ~AsyncTimer() {
            abort();
            std::lock_guard<std::mutex> guard(mtx_done);
        };

        /**
        * Run async timer thread
        */
        template<class Function, typename... Args>
        void run(Function&& func, Args ... args) {
            std::lock_guard<std::mutex> guard(mtx_done);
            isRunning = true;
            long delay_ms = 0;
            {
                mtx_params.lock();
                delay_ms = delay / 1000L;
                mtx_params.unlock();
            }

            if (delay_ms) {
                std::unique_lock<std::mutex> lk(mtx_cancel);
                cv.wait_for(lk, std::chrono::milliseconds(delay_ms));
                if (!isRunning)
                {
                    timer_thread_done.store(true);
                    // unlocking of mtx_done is done here and it is safe to destroy the object
                    return;
                }
            }

            // Start the main loop
            while (isRunning)
            {
                long interval_ms = 0;
                {
                    func(args...);
                    onTimerElapsed();

                    // lock params only for short duration
                    mtx_params.lock();
                    if (interval <= 0)
                    {
                        // interval is invalid - do not repeat, abort the task
                        isRunning = false;
                        mtx_params.unlock();
                        break;
                    }
                    // sleep with 1s wake-ups for graceful timer cancellation
                    interval_ms = interval / 1000L;
                    startTime = std::chrono::system_clock::now();
                    // expected end time
                    endTime = startTime + std::chrono::milliseconds(interval_ms);
                    mtx_params.unlock();
                }

                std::unique_lock<std::mutex> lk(mtx_cancel);

restart_timer:
                if (cv.wait_for(lk, std::chrono::milliseconds(interval_ms)) == std::cv_status::no_timeout)
                {
                    auto new_interval_ms = interval / 1000L;
                    if ((isRunning) && (new_interval_ms != interval_ms))
                    {
                        interval_ms = new_interval_ms;
                        if (interval_ms <= 0)
                        {
                            // New requested timer duration is less or eq zero. Someone asked us to switch to
                            // oneshot timer? Immediately fire the callback once and exit from the loop at
                            // LINE 108: if (interval <= 0)
                            continue;
                        }
                        // We woke up abruptly because of timer interval change.
                        // Restart with new interval and don't fire the callback yet.
                        goto restart_timer;
                    }
                }
                endTime = std::chrono::system_clock::now();
                // Repeat the job if still running
            }
            timer_thread_done.store(true);
            // unlocking of mtx_done is done here and it is safe to destroy the object
        }

        /**
         * This function does nothing and reserved for future use
         */
        static void  onTimerCallback(AsyncTimer *timer) {
            // printf("onTimerCallback: timer=%p\n", timer);
        };

        /**
         * This function has to be overridden by child class
         */
        virtual void onTimerElapsed() { };

        virtual bool start(long delay, long interval, void* callback, void* userData) {
            setDelay(delay);
            setInterval(interval);
            if (isRunning) {
                return false;
            }

            isRunning = true;
            {
                mtx_params.lock();
                // TODO: add setters for these two members
                this->callback = callback;
                this->data = userData;
                mtx_params.unlock();
            }

            if (!timer_thread_done.load())
            {
                // Timer is either already running or not stopped properly yet, so the new thread is not created.
                return false;
            }

            // Capture this, func and args and pass to run(...) method
            timer_thread_done.store(false);
            timer_thread = std::thread([this]() {
                // printf("AsyncTimer.start %p tid=%d\n", this, std::this_thread::get_id());
                run(onTimerCallback, this);
                timer_thread_done.store(true);
            });
            timer_thread.detach();
            return (timer_thread.joinable());
        }

        /**
        * Stop timer async, but don't wait for thread completion
        */
        virtual void stop()
        {
            if (isRunning) {
                isRunning = false;
            }
            cv.notify_one();
        }

        /**
        * Stop timer sync and wait for thread completion
        */
        void abort()
        {
            // printf("timer %p: abort           (done=%d)\n", this, timer_thread_done.load());
            try {
                stop();
                if (!timer_thread_done.load()) {
                    if (timer_thread.get_id() != std::this_thread::get_id()) {
                        if (timer_thread.joinable()) {
                            timer_thread.join();
                        }
                    }
                }
                // printf("timer %p: abort complete  (done=%d)\n", this, timer_thread_done.load() );
            }
            catch (...) {
                // It may occur that thread completes when we try to join
            }
        }

        /**
        * Adjust timer interval
        */
        void setInterval(long interval) {
            mtx_params.lock();
            this->interval = interval;
            if (isRunning)
            {
                // force run to goto restart_timer
                cv.notify_one();
            }
            mtx_params.unlock();
        }

        /**
        * Adjust timer delay for each iteration
        */
        void setDelay(long delay) {
            mtx_params.lock();
            this->delay = delay;
            mtx_params.unlock();
        }

    };
}

#endif


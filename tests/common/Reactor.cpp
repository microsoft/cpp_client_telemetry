//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "Reactor.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace SocketTools {

    void Reactor::addSocket(const Socket& socket, int flags)
    {
        if(flags == 0)
        {
            removeSocket(socket);
        } else
        {
            auto it = std::find(m_sockets.begin(), m_sockets.end(), socket);
            if(it == m_sockets.end())
            {
                LOG_TRACE("Reactor: Adding socket 0x%x with flags 0x%x", static_cast<int>(socket), flags);
#ifdef _WIN32
                m_events.push_back(::WSACreateEvent());
#endif
#ifdef __linux__
                epoll_event event = {};
                event.data.fd = socket;
                event.events = 0;
                ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, socket, &event);
#endif
#ifdef TARGET_OS_MAC
                struct kevent event;
                bzero(&event, sizeof(event));
                event.ident = socket.m_sock;
                EV_SET(&event, event.ident, EVFILT_READ, EV_ADD, 0, 0, NULL);
                kevent(kq, &event, 1, NULL, 0, NULL);
                EV_SET(&event, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                kevent(kq, &event, 1, NULL, 0, NULL);
#endif
                m_sockets.push_back(SocketData());
                m_sockets.back().socket = socket;
                m_sockets.back().flags = 0;
                it = m_sockets.end() - 1;
            } else
            {
                LOG_TRACE("Reactor: Updating socket 0x%x with flags 0x%x", static_cast<int>(socket), flags);
            }

            if(it->flags != flags)
            {
                it->flags = flags;
#ifdef _WIN32
                long lNetworkEvents = 0;
                if (it->flags & Readable)
                {
                    lNetworkEvents |= FD_READ;
                }
                if (it->flags & Writable)
                {
                    lNetworkEvents |= FD_WRITE;
                }
                if (it->flags & Acceptable)
                {
                    lNetworkEvents |= FD_ACCEPT;
                }
                if (it->flags & Closed)
                {
                    lNetworkEvents |= FD_CLOSE;
                }
                auto eventIt = m_events.begin() + std::distance(m_sockets.begin(), it);
                ::WSAEventSelect(socket, *eventIt, lNetworkEvents);
#endif
#ifdef __linux__
                int events = 0;
                if (it->flags & Readable)
                {
                    events |= EPOLLIN;
                }
                if (it->flags & Writable)
                {
                    events |= EPOLLOUT;
                }
                if (it->flags & Acceptable)
                {
                    events |= EPOLLIN;
                }
                // if (it->flags & Closed) - always handled (EPOLLERR | EPOLLHUP)
                epoll_event event = {};
                event.data.fd = socket;
                event.events = events;
                ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, socket, &event);
#endif
#ifdef TARGET_OS_MAC
                // TODO: [MG] - Mac OS X socket doesn't currently support updating flags
#endif
            }
        }
    }

    void Reactor::removeSocket(const Socket& socket)
    {
        LOG_TRACE("Reactor: Removing socket 0x%x", static_cast<int>(socket));
        auto it = std::find(m_sockets.begin(), m_sockets.end(), socket);
        if(it != m_sockets.end())
        {
#ifdef _WIN32
            auto eventIt = m_events.begin() + std::distance(m_sockets.begin(), it);
            ::WSAEventSelect(it->socket, *eventIt, 0);
            ::WSACloseEvent(*eventIt);
            m_events.erase(eventIt);
#endif
#ifdef __linux__
            ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, socket, nullptr);
#endif
#ifdef TARGET_OS_MAC
            struct kevent event;
            bzero(&event, sizeof(event));
            event.ident = socket;
            EV_SET(&event, socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (-1 == kevent(kq, &event, 1, NULL, 0, NULL))
            {
                //// Already removed?
                LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
            }
            EV_SET(&event, socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            if (-1 == kevent(kq, &event, 1, NULL, 0, NULL))
            {
                //// Already removed?
                LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
            }
#endif
            m_sockets.erase(it);
        }
    }

    void Reactor::start()
    {
        LOG_INFO("Reactor: Starting...");
        startThread();
    }

    void Reactor::stop()
    {
        LOG_INFO("Reactor: Stopping...");
        joinThread();
#ifdef _WIN32
        for (auto& hEvent : m_events)
        {
            ::WSACloseEvent(hEvent);
        }
#else /* Linux and Mac */
        for (auto& sd : m_sockets)
        {
#ifdef __linux__
            ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, sd.socket, nullptr);
#endif
#ifdef TARGET_OS_MAC
            struct kevent event;
            bzero(&event, sizeof(event));
            event.ident = sd.socket;
            EV_SET(&event, sd.socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (-1 == kevent(kq, &event, 1, NULL, 0, NULL))
            {
                LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
            }
            EV_SET(&event, sd.socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            if (-1 == kevent(kq, &event, 1, NULL, 0, NULL))
            {
                LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
            }
#endif
        }
#endif
        m_sockets.clear();
    }

    void Reactor::onThread()
    {
        LOG_INFO("Reactor: Thread started");
        while(!shouldTerminate())
        {
#ifdef _WIN32
            DWORD dwResult = ::WSAWaitForMultipleEvents(static_cast<DWORD>(m_events.size()), m_events.data(), FALSE, 500, FALSE);
            if (dwResult == WSA_WAIT_TIMEOUT)
            {
                continue;
            }

            assert(dwResult <= WSA_WAIT_EVENT_0 + m_events.size());
            int index = dwResult - WSA_WAIT_EVENT_0;
            Socket socket = m_sockets[index].socket;
            int flags = m_sockets[index].flags;

            WSANETWORKEVENTS ne;
            ::WSAEnumNetworkEvents(socket, m_events[index], &ne);
            LOG_TRACE("Reactor: Handling socket 0x%x (index %d) with active flags 0x%x (armed 0x%x)",
                static_cast<int>(socket), index, ne.lNetworkEvents, flags);

            if ((flags & Readable) && (ne.lNetworkEvents & FD_READ))
            {
                m_callback.onSocketReadable(socket);
            }
            if ((flags & Writable) && (ne.lNetworkEvents & FD_WRITE))
            {
                m_callback.onSocketWritable(socket);
            }
            if ((flags & Acceptable) && (ne.lNetworkEvents & FD_ACCEPT))
            {
                m_callback.onSocketAcceptable(socket);
            }
            if ((flags & Closed) && (ne.lNetworkEvents & FD_CLOSE))
            {
                m_callback.onSocketClosed(socket);
            }
#endif

#ifdef __linux__
            epoll_event events[4];
            int result = ::epoll_wait(m_epollFd, events, sizeof(events) / sizeof(events[0]), 500);
            if (result == 0 || (result == -1 && errno == EINTR))
            {
                continue;
            }

            assert(result >= 1 && static_cast<size_t>(result) <= sizeof(events) / sizeof(events[0]));
            for (int i = 0; i < result; i++)
            {
                auto it = std::find(m_sockets.begin(), m_sockets.end(), events[i].data.fd);
                assert(it != m_sockets.end());
                Socket socket = it->socket;
                int flags = it->flags;

                LOG_TRACE("Reactor: Handling socket 0x%x active flags 0x%x (armed 0x%x)",
                    static_cast<int>(socket), events[i].events, flags);

                if ((flags & Readable) && (events[i].events & EPOLLIN))
                {
                    m_callback.onSocketReadable(socket);
                }
                if ((flags & Writable) && (events[i].events & EPOLLOUT))
                {
                    m_callback.onSocketWritable(socket);
                }
                if ((flags & Acceptable) && (events[i].events & EPOLLIN))
                {
                    m_callback.onSocketAcceptable(socket);
                }
                if ((flags & Closed) && (events[i].events & (EPOLLHUP | EPOLLERR)))
                {
                    m_callback.onSocketClosed(socket);
                }
            }
#endif

#if defined(TARGET_OS_MAC)
            unsigned waitms = 500;  // never block for more than 500ms
            struct timespec timeout;
            timeout.tv_sec = waitms / 1000;
            timeout.tv_nsec = (waitms % 1000) * 1000 * 1000;

            int nev = kevent(kq, NULL, 0, m_events, KQUEUE_SIZE, &timeout);
            for(int i = 0; i < nev; i++)
            {
                struct kevent& event = m_events[i];
                int fd = (int)event.ident;
                auto it = std::find(m_sockets.begin(), m_sockets.end(), fd);
                assert(it != m_sockets.end());
                Socket socket = it->socket;
                int flags = it->flags;

                LOG_TRACE("Handling socket 0x%x active flags 0x%x (armed 0x%x)",
                    static_cast<int>(socket), event.flags, event.fflags);

                if (event.filter==EVFILT_READ)
                {
                    if (flags & Acceptable)
                    {
                        m_callback.onSocketAcceptable(socket);
                    }
                    if (flags & Readable)
                    {
                        m_callback.onSocketReadable(socket);
                    }
                    continue;
                }

                if (event.filter==EVFILT_WRITE)
                {
                    if (flags & Writable)
                    {
                        m_callback.onSocketWritable(socket);
                    }
                    continue;
                }

                if ((event.flags & EV_EOF)||(event.flags & EV_ERROR))
                {
                    LOG_TRACE("event.filter=%s", "EVFILT_WRITE");
                    m_callback.onSocketClosed(socket);
                    it->flags=Closed;
                    struct kevent kevt;
                    EV_SET(&kevt, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if (-1 == kevent(kq, &kevt, 1, NULL, 0, NULL))
                    {
                        LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
                    }
                    EV_SET(&kevt, event.ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
                    if (-1 == kevent(kq, &kevt, 1, NULL, 0, NULL))
                    {
                        LOG_ERROR("cannot delete fd=0x%x from kqueue!", event.ident);
                    }
                    continue;
                }
                LOG_ERROR("Reactor: unhandled kevent!");
            }
#endif
        }
        LOG_TRACE("Reactor: Thread done");
    };

}


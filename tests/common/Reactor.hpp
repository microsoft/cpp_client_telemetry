//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef REACTOR_HPP
#define REACTOR_HPP

#include "SocketTools.hpp"

namespace SocketTools {

class Reactor : protected Thread
{

 public:

    class Callback
    {
     public:
        virtual void onSocketReadable(Socket sock) = 0;
        virtual void onSocketWritable(Socket sock) = 0;
        virtual void onSocketAcceptable(Socket sock) = 0;
        virtual void onSocketClosed(Socket sock) = 0;
    };

    enum State
    {
        Readable = 1,
        Writable = 2,
        Acceptable = 4,
        Closed = 8
    };

    void addSocket(const Socket& socket, int flags);
    void removeSocket(const Socket& socket);
    void start();
    void stop();

    virtual void onThread() override;

 protected:

    Callback& m_callback;
    std::vector<SocketData> m_sockets;

#ifdef _WIN32
    /* use WinSock events on Windows */
    std::vector<WSAEVENT> m_events;
#endif
#ifdef __linux__
    /* use epoll on Linux */
    int m_epollFd;
#endif
#ifdef TARGET_OS_MAC
    /* use kqueue on Mac */
#define KQUEUE_SIZE     32
    int kq { 0 };
    struct kevent m_events[KQUEUE_SIZE];
#endif

 public:

    Reactor(Callback& callback) : m_callback(callback)
    {
#ifdef __linux__
#ifdef ANDROID
        m_epollFd = ::epoll_create(0);
#else
        m_epollFd = ::epoll_create1(0);
#endif
#endif
#ifdef TARGET_OS_MAC
        bzero(&m_events[0], sizeof(m_events));
        kq = kqueue();
#endif
    }

    ~Reactor()
    {
#ifdef __linux__
        ::close(m_epollFd);
#endif
#ifdef TARGET_OS_MAC
        ::close(kq);
#endif
    }

}; /* end of class Reactor */

}

#endif



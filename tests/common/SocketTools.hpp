// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Common.hpp"
// *INDENT-OFF* special formatting here for clarity

#ifdef _WIN32
#include <winsock2.h>
#undef min
#undef max
#pragma comment(lib, "ws2_32.lib")

namespace testing {

    // WinSocks need extra (de)initialization, solved by a global object here,
    // whose constructor/destructor will be called before and after main().
    class WsaInitializer
    {
    public:
        WsaInitializer()
        {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
        }

        ~WsaInitializer()
        {
            WSACleanup();
        }
    };
    static WsaInitializer g_wsaInitializer;

} // namespace testing

// For _beginthread() etc.
#include <process.h>
#else

#ifdef __linux__
#include <sys/epoll.h>
#else
#if __APPLE__
// Use kqueue on mac
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif
#endif

// Common POSIX headers for Linux and Mac OS X
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#endif // end of network headers

#ifndef _Out_cap_
#define _Out_cap_(size)
#endif

// *INDENT-OFF*

namespace testing {

// Encapsulation of sockaddr(_in)
    class SocketAddr
    {
    public:
        static u_long const Loopback = 0x7F000001;

    protected:
        sockaddr m_data;

    public:
        SocketAddr()
        {
            memset(&m_data, 0, sizeof(m_data));
        }

        SocketAddr(u_long addr, int port)
        {
            sockaddr_in& inet4 = reinterpret_cast<sockaddr_in&>(m_data);
            inet4.sin_family = AF_INET;
            inet4.sin_port = htons(static_cast<unsigned short>(port));
            inet4.sin_addr.s_addr = htonl(addr);
        }

        SocketAddr(char const* addr)
        {
#ifdef _WIN32
            INT addrlen = sizeof(m_data);
            WCHAR buf[200];
            for (int i = 0; i < sizeof(buf) && addr[i]; i++) {
                buf[i] = addr[i];
            }
            buf[199] = L'\0';
            ::WSAStringToAddressW(buf, AF_INET, nullptr, &m_data, &addrlen);
#else
            sockaddr_in& inet4 = reinterpret_cast<sockaddr_in&>(m_data);
            inet4.sin_family = AF_INET;
            char const* colon = strchr(addr, ':');
            if (colon) {
                inet4.sin_port = htons(atoi(colon + 1));
                char buf[16];
                memcpy(buf, addr, std::min<ptrdiff_t>(15, colon - addr));
                buf[15] = '\0';
                ::inet_pton(AF_INET, buf, &inet4.sin_addr);
            }
            else {
                inet4.sin_port = 0;
                ::inet_pton(AF_INET, addr, &inet4.sin_addr);
            }
#endif
        }

        SocketAddr(SocketAddr const& other) = default;

        SocketAddr& operator=(SocketAddr const& other) = default;

        operator sockaddr* ()
        {
            return &m_data;
        }

        operator const sockaddr* () const
        {
            return &m_data;
        }

        int port() const
        {
            switch (m_data.sa_family) {
            case AF_INET: {
                sockaddr_in const& inet4 = reinterpret_cast<sockaddr_in const&>(m_data);
                return ntohs(inet4.sin_port);
            }

            default:
                return -1;
            }
        }

        std::string toString() const
        {
            std::ostringstream os;

            switch (m_data.sa_family) {
            case AF_INET: {
                sockaddr_in const& inet4 = reinterpret_cast<sockaddr_in const&>(m_data);
                u_long addr = ntohl(inet4.sin_addr.s_addr);
                os << (addr >> 24) << '.' << ((addr >> 16) & 255) << '.' << ((addr >> 8) & 255) << '.' << (addr & 255);
                os << ':' << ntohs(inet4.sin_port);
                break;
            }

            default:
                os << "[?AF?" << m_data.sa_family << ']';
            }

            return os.str();
        }
    };

    //---

    // Encapsulation of a socket (non-exclusive ownership)
    class Socket
    {
    public:
#ifdef _WIN32
        typedef SOCKET Type;
        static Type const Invalid = INVALID_SOCKET;
#else
        typedef int Type;
        static Type const Invalid = -1;
#endif

    protected:
        Type m_sock;

    public:
        Socket(Type sock = Invalid)
            : m_sock(sock)
        {
        }

        Socket(int af, int type, int proto)
        {
            m_sock = ::socket(af, type, proto);
        }

        ~Socket()
        {
        }

        operator Socket::Type() const
        {
            return m_sock;
        }

        bool operator==(Socket const& other) const
        {
            return (m_sock == other.m_sock);
        }

        bool operator!=(Socket const& other) const
        {
            return (m_sock != other.m_sock);
        }

        bool operator<(Socket const& other) const
        {
            return (m_sock < other.m_sock);
        }

        bool invalid() const
        {
            return (m_sock == Invalid);
        }

        void setNonBlocking()
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            u_long value = 1;
            ::ioctlsocket(m_sock, FIONBIO, &value);
#else
            int flags = ::fcntl(m_sock, F_GETFL, 0);
            ::fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);
#endif
        }

        bool setReuseAddr()
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            BOOL value = TRUE;
#else
            int value = 1;
#endif
            return (::setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&value), sizeof(value)) == 0);
        }

        bool setNoDelay()
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            BOOL value = TRUE;
#else
            int value = 1;
#endif
            return (::setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value)) == 0);
        }

        bool connect(SocketAddr const& addr)
        {
            assert(m_sock != Invalid);
            return (::connect(m_sock, addr, sizeof(addr)) == 0);
        }

        void close()
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            ::closesocket(m_sock);
#else
            ::close(m_sock);
#endif
            m_sock = Invalid;
        }

        int recv(_Out_cap_(size) void* buffer, unsigned size)
        {
            assert(m_sock != Invalid);
            return ::recv(m_sock, reinterpret_cast<char*>(buffer), size, 0);
        }

        int send(void const* buffer, unsigned size)
        {
            assert(m_sock != Invalid);
            return ::send(m_sock, reinterpret_cast<char const*>(buffer), size, 0);
        }

        bool bind(SocketAddr const& addr)
        {
            assert(m_sock != Invalid);
            return (::bind(m_sock, addr, sizeof(addr)) == 0);
        }

        bool getsockname(SocketAddr& addr) const
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            int addrlen = sizeof(addr);
#else
            socklen_t addrlen = sizeof(addr);
#endif
            return (::getsockname(m_sock, addr, &addrlen) == 0);
        }

        bool listen(int backlog)
        {
            assert(m_sock != Invalid);
            return (::listen(m_sock, backlog) == 0);
        }

        bool accept(Socket& csock, SocketAddr& caddr)
        {
            assert(m_sock != Invalid);
#ifdef _WIN32
            int addrlen = sizeof(caddr);
#else
            socklen_t addrlen = sizeof(caddr);
#endif
            csock = ::accept(m_sock, caddr, &addrlen);
            return !csock.invalid();
        }

        bool shutdown(int how)
        {
            assert(m_sock != Invalid);
            return (::shutdown(m_sock, how) == 0);
        }

        int error() const
        {
#ifdef _WIN32
            return ::WSAGetLastError();
#else
            return errno;
#endif
        }

        enum {
#ifdef _WIN32
            ErrorWouldBlock = WSAEWOULDBLOCK
#else
            ErrorWouldBlock = EWOULDBLOCK
#endif
        };

        enum {
#ifdef _WIN32
            ShutdownReceive = SD_RECEIVE, ShutdownSend = SD_SEND, ShutdownBoth = SD_BOTH
#else
            ShutdownReceive = SHUT_RD, ShutdownSend = SHUT_WR, ShutdownBoth = SHUT_RDWR
#endif
        };
    };

    //---

    // A simple thread, derived class overloads onThread() method
    class Thread
    {
    private:
#ifdef _WIN32
        uintptr_t m_thread;
#else
        pthread_t m_thread;
#endif
        volatile bool m_terminate;

    protected:
        Thread()
        {
        }

        void startThread()
        {
            m_terminate = false;
#ifdef _WIN32
            m_thread = _beginthreadex(nullptr, 0, &threadFunc, static_cast<void*>(this), 0, nullptr);
#else
            pthread_create(&m_thread, nullptr, &threadFunc, static_cast<void*>(this));
#endif
        }

        void joinThread()
        {
            m_terminate = true;
#ifdef _WIN32
            WaitForSingleObject(reinterpret_cast<HANDLE>(m_thread), INFINITE);
#else
            void* result;
            pthread_join(m_thread, &result);
#endif
        }

        bool shouldTerminate() const
        {
            return m_terminate;
        }

        virtual void onThread() = 0;

    private:
#ifdef _WIN32
        static unsigned __stdcall threadFunc(void* param)
#else
        static void* threadFunc(void* param)
#endif
        {
            static_cast<Thread*>(param)->onThread();
            return 0;
        }
    };

    //---

//#if defined(_WIN32) || defined(__linux__)
    // Reactor for asynchronous watching of sockets
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

        enum State { Readable = 1, Writable = 2, Acceptable = 4, Closed = 8 };

    protected:
        struct SocketData {
            Socket socket;
            int flags;

            SocketData()
                : socket(),
                flags(0)
            {
            }

            bool operator==(Socket s)
            {
                return (socket == s);
            }
        };

        Callback& m_callback;
        std::vector<SocketData> m_sockets;
#ifdef _WIN32
        std::vector<WSAEVENT> m_events;
#endif

#ifdef __linux__
        int m_epollFd;
#endif

#ifdef __APPLE__
        int kq;
#endif

    public:
        Reactor(Callback& callback)
            : m_callback(callback)
        {
#ifdef __linux__
            m_epollFd = ::epoll_create1(0);
#endif
#ifdef __APPLE__
            kq = kqueue();
#endif
        }

        ~Reactor()
        {
#ifdef __linux__
//#ifndef _WIN32
            ::close(m_epollFd);
#endif
#ifdef __APPLE__
            close(kq);
#endif
        }

        void addSocket(const Socket& socket, int flags)
        {
            if (flags == 0) {
                removeSocket(socket);
            }
            else {
                auto it = std::find(m_sockets.begin(), m_sockets.end(), socket);
                if (it == m_sockets.end()) {
                    LOG_TRACE("Reactor: Adding socket %d with flags %d", static_cast<int>(socket), flags);
#ifdef _WIN32
                    m_events.push_back(::WSACreateEvent());
#endif

#ifdef __linux__
                    epoll_event event = {};
                    event.data.fd = socket;
                    event.events = 0;
                    ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, socket, &event);
#endif
#ifdef __APPLE__
                    struct kevent event;
                    event.ident = socket;
#endif

// TODO: Mac OS X socket polling using kqueue

                    m_sockets.push_back(SocketData());
                    m_sockets.back().socket = socket;
                    m_sockets.back().flags = 0;
                    it = m_sockets.end() - 1;
                }
                else {
                    LOG_TRACE("Reactor: Updating socket %d with flags %d", static_cast<int>(socket), flags);
                }

                if (it->flags != flags) {
                    it->flags = flags;
#ifdef _WIN32
                    long lNetworkEvents = 0;
                    if (it->flags & Readable) {
                        lNetworkEvents |= FD_READ;
                    }
                    if (it->flags & Writable) {
                        lNetworkEvents |= FD_WRITE;
                    }
                    if (it->flags & Acceptable) {
                        lNetworkEvents |= FD_ACCEPT;
                    }
                    if (it->flags & Closed) {
                        lNetworkEvents |= FD_CLOSE;
                    }
                    auto eventIt = m_events.begin() + std::distance(m_sockets.begin(), it);
                    ::WSAEventSelect(socket, *eventIt, lNetworkEvents);
#endif

#ifdef __linux__
                    int events = 0;
                    if (it->flags & Readable) {
                        events |= EPOLLIN;
                    }
                    if (it->flags & Writable) {
                        events |= EPOLLOUT;
                    }
                    if (it->flags & Acceptable) {
                        events |= EPOLLIN;
                    }
                    // if (it->flags & Closed) - always handled (EPOLLERR | EPOLLHUP)
                    epoll_event event = {};
                    event.data.fd = socket;
                    event.events = events;
                    ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, socket, &event);
#endif

// TODO: Mac OS X socket accepting implementation
                }
            }
        }

        void removeSocket(const Socket& socket)
        {
            LOG_TRACE("Reactor: Removing socket %d", static_cast<int>(socket));
            auto it = std::find(m_sockets.begin(), m_sockets.end(), socket);
            if (it != m_sockets.end()) {
#ifdef _WIN32
                auto eventIt = m_events.begin() + std::distance(m_sockets.begin(), it);
                ::WSAEventSelect(it->socket, *eventIt, 0);
                ::WSACloseEvent(*eventIt);
                m_events.erase(eventIt);
#endif

#ifdef __linux__
                ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, socket, nullptr);
#endif

                m_sockets.erase(it);
            }
        }

        void start()
        {
            LOG_INFO("Reactor: Starting...");
            startThread();
        }

        void stop()
        {
            LOG_INFO("Reactor: Stopping...");
            joinThread();

#ifdef _WIN32
            for (auto& hEvent : m_events) {
                ::WSACloseEvent(hEvent);
            }
#endif

#ifdef __linux__
            for (auto& sd : m_sockets) {
                ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, sd.socket, nullptr);
            }
#endif

            m_sockets.clear();
        }

    protected:
        virtual void onThread() override
        {
            LOG_INFO("Reactor: Thread started");
            while (!shouldTerminate()) {
#ifdef _WIN32
                DWORD dwResult = ::WSAWaitForMultipleEvents(static_cast<DWORD>(m_events.size()), m_events.data(), FALSE, 500, FALSE);
                if (dwResult == WSA_WAIT_TIMEOUT) {
                    continue;
                }

                assert(dwResult <= WSA_WAIT_EVENT_0 + m_events.size());
                int index = dwResult - WSA_WAIT_EVENT_0;
                Socket socket = m_sockets[index].socket;
                int flags = m_sockets[index].flags;

                WSANETWORKEVENTS ne;
                ::WSAEnumNetworkEvents(socket, m_events[index], &ne);
                LOG_TRACE("Reactor: Handling socket %d (index %d) with active flags %d (armed %d)", static_cast<int>(socket), index, ne.lNetworkEvents, flags);

                if ((flags & Readable) && (ne.lNetworkEvents & FD_READ)) {
                    m_callback.onSocketReadable(socket);
                }
                if ((flags & Writable) && (ne.lNetworkEvents & FD_WRITE)) {
                    m_callback.onSocketWritable(socket);
                }
                if ((flags & Acceptable) && (ne.lNetworkEvents & FD_ACCEPT)) {
                    m_callback.onSocketAcceptable(socket);
                }
                if ((flags & Closed) && (ne.lNetworkEvents & FD_CLOSE)) {
                    m_callback.onSocketClosed(socket);
                }
#endif


#ifdef __linux__
                epoll_event events[4];
                int result = ::epoll_wait(m_epollFd, events, sizeof(events) / sizeof(events[0]), 500);
                if (result == 0 || (result == -1 && errno == EINTR)) {
                    continue;
                }

                assert(result >= 1 && static_cast<size_t>(result) <= sizeof(events) / sizeof(events[0]));
                for (int i = 0; i < result; i++) {
                    auto it = std::find(m_sockets.begin(), m_sockets.end(), events[i].data.fd);
                    assert(it != m_sockets.end());
                    Socket socket = it->socket;
                    int flags = it->flags;

                    LOG_TRACE("Reactor: Handling socket %d active flags %d (armed %d)", static_cast<int>(socket), events[i].events, flags);

                    if ((flags & Readable) && (events[i].events & EPOLLIN)) {
                        m_callback.onSocketReadable(socket);
                    }
                    if ((flags & Writable) && (events[i].events & EPOLLOUT)) {
                        m_callback.onSocketWritable(socket);
                    }
                    if ((flags & Acceptable) && (events[i].events & EPOLLIN)) {
                        m_callback.onSocketAcceptable(socket);
                    }
                    if ((flags & Closed) && (events[i].events & (EPOLLHUP | EPOLLERR))) {
                        m_callback.onSocketClosed(socket);
                    }
                }
#endif

// TODO: provide Mac OS X polling implementation using kqueue

            }
            LOG_INFO("Reactor: Thread done");
        }
    };

} // namespace testing

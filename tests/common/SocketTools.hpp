//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef SOCKETTOOLS_HPP
#define SOCKETTOOLS_HPP

#include <vector>
#include <string>
#include <thread>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstddef>
#include <algorithm>

#if defined(HAVE_CONSOLE_LOG) && !defined(LOG_DEBUG)
/* Log to console if there's no standard log facility defined */
#  include <cstdio>
#  ifndef LOG_DEBUG
#    define LOG_DEBUG(fmt_, ...)    printf(" " fmt_ "\n", ##__VA_ARGS__)
#    define LOG_TRACE(fmt_, ...)    printf(" " fmt_ "\n", ##__VA_ARGS__)
#    define LOG_INFO(fmt_, ...)     printf(" " fmt_ "\n", ##__VA_ARGS__)
#    define LOG_WARN(fmt_, ...)     printf(" " fmt_ "\n", ##__VA_ARGS__)
#    define LOG_ERROR(fmt_, ...)    printf(" " fmt_ "\n", ##__VA_ARGS__)
#  endif
#endif

#ifndef LOG_DEBUG
/* Don't log anything if there's no standard log facility defined */
#  define LOG_DEBUG(fmt_, ...)
#  define LOG_TRACE(fmt_, ...)
#  define LOG_INFO(fmt_, ...)
#  define LOG_WARN(fmt_, ...)
#  define LOG_ERROR(fmt_, ...)
#endif

#ifdef _WIN32
#include <winsock2.h>
#undef min
#undef max
#pragma comment(lib, "ws2_32.lib")

namespace SocketTools {

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

}
#else

#include <unistd.h>

#ifdef __linux__
#include <sys/epoll.h>
#endif

#if __APPLE__
#include "TargetConditionals.h"
// Use kqueue on mac
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

// Common POSIX headers for Linux and Mac OS X
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#endif

#ifndef _Out_cap_
#define _Out_cap_(size)
#endif

namespace SocketTools {

/**
 * Encapsulation of sockaddr(_in)
 */
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
        for(int i = 0; i < sizeof(buf) && addr[i]; i++)
        {
            buf[i] = addr[i];
        }
        buf[199] = L'\0';
        ::WSAStringToAddressW(buf, AF_INET, nullptr, &m_data, &addrlen);
#else
        sockaddr_in& inet4 = reinterpret_cast<sockaddr_in&>(m_data);
        inet4.sin_family = AF_INET;
        char const* colon = strchr(addr, ':');
        if(colon)
        {
            inet4.sin_port = htons(atoi(colon + 1));
            char buf[16];
            memcpy(buf, addr, std::min<ptrdiff_t>(15, colon - addr));
            buf[15] = '\0';
            ::inet_pton(AF_INET, buf, &inet4.sin_addr);
        } else
        {
            inet4.sin_port = 0;
            ::inet_pton(AF_INET, addr, &inet4.sin_addr);
        }
#endif
    }

    SocketAddr(SocketAddr const& other) = default;

    SocketAddr& operator=(SocketAddr const& other) = default;

    operator sockaddr*()
    {
        return &m_data;
    }

    operator const sockaddr*() const
    {
        return &m_data;
    }

    int port() const
    {
        switch(m_data.sa_family)
        {
            case AF_INET:
            {
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

        switch(m_data.sa_family)
        {
            case AF_INET:
            {
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

/**
 * Encapsulation of a socket (non-exclusive ownership)
 */
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
        int flags = 0;
        return static_cast<int>(::recv(m_sock, reinterpret_cast<char*>(buffer), size, flags));
    }

    int send(void const* buffer, unsigned size)
    {
        assert(m_sock != Invalid);
        return static_cast<int>(::send(m_sock, reinterpret_cast<char const*>(buffer), size, 0));
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

    enum
    {
#ifdef _WIN32
        ErrorWouldBlock = WSAEWOULDBLOCK
#else
        ErrorWouldBlock = EWOULDBLOCK
#endif
    };

    enum
    {
#ifdef _WIN32
        ShutdownReceive = SD_RECEIVE,
        ShutdownSend = SD_SEND,
        ShutdownBoth = SD_BOTH
#else
        ShutdownReceive = SHUT_RD,
        ShutdownSend = SHUT_WR,
        ShutdownBoth = SHUT_RDWR
#endif
    };
};

/**
 * A simple thread, derived class overloads onThread() method
 */
class Thread
{
 private:
    std::thread m_thread;
    volatile bool m_terminate { false };

 protected:
    Thread()
    {
    }

    void startThread()
    {
        m_terminate = false;
        m_thread = std::thread(
        [&]()
        {
            this->onThread();
        });
    }

    void joinThread()
    {
        m_terminate = true;
        if(m_thread.joinable())
        {
            m_thread.join();
        }
    }

    bool shouldTerminate() const
    {
        return m_terminate;
    }

    virtual void onThread() = 0;

    virtual ~Thread() noexcept
    {
    }

};

struct SocketData
{
    Socket socket;
    int flags;

    SocketData() : socket(), flags(0)
    {
    }

    bool operator==(Socket s)
    {
        return (socket == s);
    }
};

}
#endif



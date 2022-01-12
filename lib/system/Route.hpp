//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef SYSTEM_ROUTE_HPP
#define SYSTEM_ROUTE_HPP

#include "pal/PAL.hpp"

#include <assert.h>
#include <vector>

namespace MAT_NS_BEGIN {

    //! Interface for generic route sink (incoming data handler)
    template<typename... TArgs>
    class IRouteSink {
    public:
        virtual ~IRouteSink() {}
        virtual void operator()(TArgs... args) = 0;
        using ReturnType = void;
    };


    //! Interface for generic route pass-through (returns false to stop the flow)
    template<typename... TArgs>
    class IRoutePassThrough {
    public:
        virtual ~IRoutePassThrough() {}
        virtual bool operator()(TArgs... args) = 0;
        using ReturnType = bool;
    };


    //! Helper - IRouteSink/IRoutePassThrough implementation over member functor
    template<typename TParent, typename TOwner, typename... TArgs>
    class RouteHandlerT : public TParent {
    public:
        RouteHandlerT(TOwner* owner, typename TParent::ReturnType(TOwner::* handler)(TArgs...))
            : m_owner(owner),
            m_handler(handler)
        {
        }

        virtual typename TParent::ReturnType operator()(TArgs... args) override
        {
            return (m_owner->*m_handler)(std::forward<TArgs>(args) ...);
        }

    protected:
        TOwner * m_owner;
        typename TParent::ReturnType(TOwner::* m_handler)(TArgs...);
    };


    //! Default route sink - calls the handler, does not continue the flow
    template<typename TOwner, typename... TArgs>
    using RouteSink = RouteHandlerT<IRouteSink<TArgs...>, TOwner, TArgs...>;


    //! Default route pass-through - calls the handler, then continues or stops
    template<typename TOwner, typename... TArgs>
    using RoutePassThrough = RouteHandlerT<IRoutePassThrough<TArgs...>, TOwner, TArgs...>;


    template<typename... TArgs>
    class RouteBuilder;


    //! Route source - call like a function to send data downstream
    template<typename... TArgs>
    class RouteSource {
    public:
        RouteSource()
            : m_target(nullptr)
        {
        }

        RouteBuilder<TArgs...> operator>>(IRoutePassThrough<TArgs...>& target)
        {
            if (m_target != nullptr || !m_passthroughs.empty()) {
                assert(!"Source already bound");
            }
            return RouteBuilder<TArgs...>(*this, target);
        }

        RouteBuilder<TArgs...> operator>>(IRouteSink<TArgs...>& target)
        {
            if (m_target != nullptr || !m_passthroughs.empty()) {
                assert(!"Source already bound");
            }
            return RouteBuilder<TArgs...>(*this, target);
        }

        void connect(std::vector<IRoutePassThrough<TArgs...>*>&& passthroughs, IRouteSink<TArgs...>* sink)
        {
            m_passthroughs = std::move(passthroughs);
            m_target = sink;
        }

        template<typename... TRealArgs>
        void operator()(TRealArgs&& ... args) const
        {
            for (IRoutePassThrough<TArgs...>* target : m_passthroughs) {
                if (!(*target)(std::forward<TRealArgs>(args) ...)) {
                    return;
                }
            }
            if (m_target) {
                (*m_target)(std::forward<TRealArgs>(args) ...);
            }
        }

    protected:
        std::vector<IRoutePassThrough<TArgs...>*> m_passthroughs;
        IRouteSink<TArgs...>*                     m_target;
    };


    //! Helper - builds route flows with operator >>
    template<typename... TArgs>
    class RouteBuilder {
    public:
        RouteBuilder(RouteSource<TArgs...>& source, IRoutePassThrough<TArgs...>& target)
            : m_source(&source),
            m_passthroughs(1, &target)
        {
        }

        RouteBuilder(RouteSource<TArgs...>& source, IRouteSink<TArgs...>& target)
            : m_source(nullptr)
        {
            source.connect({}, &target);
        }

        RouteBuilder(RouteBuilder const&) = delete;
        RouteBuilder& operator=(RouteBuilder const&) = delete;

        RouteBuilder(RouteBuilder&& other)
            : m_source(std::move(other.m_source)),
            m_passthroughs(std::move(other.m_passthroughs))
        {
            other.m_source = nullptr;
        }

        RouteBuilder& operator=(RouteBuilder&& other)
        {
            m_source = std::move(other.m_source);
            m_passthroughs = std::move(other.m_passthroughs);
            other.m_source = nullptr;
            return *this;
        }

        RouteBuilder<TArgs...> operator>>(IRouteSink<TArgs...>& target)
        {
            if (m_source != nullptr) {
                m_source->connect(std::move(m_passthroughs), &target);
                m_source = nullptr;
            }
            else {
                assert(!"Builder instance inactive (wrong temporary) or already finished (bound to a sink)");
            }
            return std::move(*this);
        }

        RouteBuilder<TArgs...> operator>>(IRoutePassThrough<TArgs...>& target)
        {
            if (m_source != nullptr) {
                m_passthroughs.push_back(&target);
            }
            else {
                assert(!"Builder instance inactive (wrong temporary) or already finished (bound to a sink)");
            }
            return std::move(*this);
        }

        ~RouteBuilder()
        {
            if (m_source != nullptr) {
                m_source->connect(std::move(m_passthroughs), nullptr);
            }
        }

    protected:
        RouteSource<TArgs...>*                    m_source;
        std::vector<IRoutePassThrough<TArgs...>*> m_passthroughs;
    };

} MAT_NS_END
#endif


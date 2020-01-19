// Copyright (c) Microsoft. All rights reserved.
//
// ISerializer.h
//
#ifndef LIB_EXPORTERS_ISERIALIZER_HPP_
#define LIB_EXPORTERS_ISERIALIZER_HPP_

#include "system/Contexts.hpp"
#include "system/Route.hpp"

#include "exporters/ISplicer.hpp"

namespace ARIASDK_NS_BEGIN
{
    class ISerializer
    {
       protected:
        virtual bool handleSerialize(IncomingEventContextPtr const& ctx) = 0;
        std::unique_ptr<ISplicer> splicer;

       public:
        ISerializer() = default;
        virtual ~ISerializer() = default;
        RoutePassThrough<ISerializer, IncomingEventContextPtr const&> serialize{this, &ISerializer::handleSerialize};
    };

}
ARIASDK_NS_END

#endif /* LIB_EXPORTERS_ISERIALIZER_HPP_ */

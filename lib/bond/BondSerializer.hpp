// Copyright (c) Microsoft. All rights reserved.
#ifndef BONDSERIALIZER_HPP
#define BONDSERIALIZER_HPP

#include "exporters/ISerializer.hpp"

namespace ARIASDK_NS_BEGIN
{
    class BondSerializer : public ISerializer
    {
       protected:
        virtual bool handleSerialize(IncomingEventContextPtr const& ctx) override;
    };

}
ARIASDK_NS_END

#endif

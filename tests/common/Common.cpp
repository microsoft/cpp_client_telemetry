// Copyright (c) Microsoft. All rights reserved.

#include "Common.hpp"

namespace testing {


ARIASDK_LOG_INST_COMPONENT_NS("Testing", "Unit testing helpers");

AriaProtocol::Value toAriaProtocolValue(std::string val)
{
    AriaProtocol::Value temp;
    temp.stringValue = val;
    return temp;
}

AriaProtocol::Value toAriaProtocolValue(bool val)
{
    AriaProtocol::Value temp;
    temp.type = AriaProtocol::ValueBool;
    temp.longValue = val;
    return temp;
}


AriaProtocol::Value toAriaProtocolValue(double val)
{
    AriaProtocol::Value temp;
    temp.type = AriaProtocol::ValueDouble;
    temp.doubleValue = val;
    return temp;
}


AriaProtocol::Value toAriaProtocolValue(int64_t val)
{
    AriaProtocol::Value temp;
    temp.type = AriaProtocol::ValueInt64;
    temp.longValue = val;
    return temp;
}

AriaProtocol::Value toAriaProtocolValue(uint64_t val)
{
    AriaProtocol::Value temp;
    temp.type = AriaProtocol::ValueArrayUInt64;
    temp.longValue = val;
    return temp;
}


AriaProtocol::Value toAriaProtocolValue(Microsoft::Applications::Telemetry::EventLatency val)
{
    AriaProtocol::Value temp;
    temp.type = AriaProtocol::ValueArrayInt32;
    temp.longValue = (int)val;
    return temp;
}



} // namespace testing

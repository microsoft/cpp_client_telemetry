// Copyright (c) Microsoft. All rights reserved.

#ifndef CPP_CLIENT_TELEMETRY_LOGGERWRAPPER_H
#define CPP_CLIENT_TELEMETRY_LOGGERWRAPPER_H
#include "Version.hpp"
#include "ILogger.hpp"
#include "EventProperties.hpp"

namespace ARIASDK_NS_BEGIN {
    class LoggerWrapper {
    private:
        ILogger *logger;
    public:
        LoggerWrapper();
        void LogEvent(EventProperties const&);
    };
} ARIASDK_NS_END
#endif //CPP_CLIENT_TELEMETRY_LOGGERWRAPPER_H
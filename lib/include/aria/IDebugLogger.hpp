// IDebugLogger.hpp
//
// Interface through which modular components log to the debug stream
//

#ifndef ARIA_IDEBUGLOGGER_HPP
#define ARIA_IDEBUGLOGGER_HPP

#include "Version.hpp"
#include "ModularShared.hpp"

namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// Interface to allow modular replacements to integrate with core debug logging
    /// </summary>
    class IDebugLogger
    {
    public:
        virtual ~IDebugLogger() {}

        /// <summary>
        /// Add a detail-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogDetail(const char* component, const char * format, ...) const ARIA_NOEXCEPT = 0;

        /// <summary>
        /// Add an info-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogInfo(const char* component, const char * format, ...) const ARIA_NOEXCEPT = 0;

        /// <summary>
        /// Add a warning-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogWarning(const char* component, const char * format, ...) const ARIA_NOEXCEPT = 0;

        /// <summary>
        /// Add an error-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogError(const char* component, const char * format, ...) const ARIA_NOEXCEPT = 0;
    };
} ARIASDK_NS_END

#endif // !ARIA_IDEBUGLOGGER_HPP

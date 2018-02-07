#include "DebugLogger.hpp"
#include "pal/PAL.hpp"

// One macro for maintenance, called from multiple methods to reduce how many
// va_list objects we need to initialize as we descend the stack.
#define LogIfEnabled(level_, component_, format_)                               \
if (ARIASDK_LOG_ENABLED_((level_)))                                             \
{                                                                               \
    va_list args;                                                               \
    va_start(args, format_);                                                    \
    ::ARIASDK_NS::PAL::detail::log((level_), (component_), (format_), args);    \
    va_end(args);                                                               \
}

namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// Implementation of IDebugLogger, simply chains to existing SDK logging engine
    /// </summary>
    class DebugLogger : public IDebugLogger
    {
    public:
        /// <summary>
        /// Add a detail-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogDetail(const char* component, const char * format, ...) const ARIA_NOEXCEPT override
        {
            LogIfEnabled(::ARIASDK_NS::PAL::Detail, component, format);
        }

        /// <summary>
        /// Add an info-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogInfo(const char* component, const char * format, ...) const ARIA_NOEXCEPT override
        {
            LogIfEnabled(::ARIASDK_NS::PAL::Info, component, format);
        }

        /// <summary>
        /// Add a warning-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogWarning(const char* component, const char * format, ...) const ARIA_NOEXCEPT override
        {
            LogIfEnabled(::ARIASDK_NS::PAL::Warning, component, format);
        }

        /// <summary>
        /// Add an error-level entry to the debug log
        /// </summary>
        /// <param name="component">The component generating the log entry</param>
        /// <param name="format">A printf-formatted string. Parameters follow</param>
        virtual void cdecl LogError(const char* component, const char * format, ...) const ARIA_NOEXCEPT override
        {
            LogIfEnabled(::ARIASDK_NS::PAL::Error, component, format);
        }
    };

    static DebugLogger _debugLogger;

    IDebugLogger* GetDebugLogger()
    {
        return &_debugLogger;
    }

} ARIASDK_NS_END


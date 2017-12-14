// Copyright (c) Microsoft. All rights reserved.
#ifndef ARIA_ILOGGER_HPP
#define ARIA_ILOGGER_HPP

//#include "Version.hpp"
#include "ctmacros.hpp"
#include "Enums.hpp"
#include "EventProperties.hpp"
#include "ISemanticContext.hpp"
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
// *INDENT-ON*

//Data Type Flags
#define MICROSOFT_KEYWORD_CRITICAL_DATA 0x0000800000000000 // Bit 47
#define MICROSOFT_KEYWORD_MEASURES      0x0000400000000000 // Bit 46
#define MICROSOFT_KEYWORD_TELEMETRY     0x0000200000000000 // Bit 45
//Core data Flags
#define MICROSOFT_EVENTTAG_CORE_DATA            0x00080000
//Latency Flags
#define MICROSOFT_EVENTTAG_COSTDEFERRED_LATENCY 0x00040000
#define MICROSOFT_EVENTTAG_REALTIME_LATENCY     0x00200000
#define MICROSOFT_EVENTTAG_NORMAL_LATENCY       0x00400000

/// <summary>
/// ILogger interface for logging either semantic or custom event
/// </summary>
class ARIASDK_LIBABI ILogger
{
  public:
    virtual ~ILogger() {}
    /// <summary>
    /// Gets an ISemanticContext interface through which to specify semantic context of this logger instance.
    /// </summary>
    /// <returns>ISemanticContext interface</returns>
    virtual ISemanticContext* GetSemanticContext() const = 0 ;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, const char value[], PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">Double value of the property</param>
	virtual void SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">64-bit Integer value of the property</param>
	virtual void SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">String value of the property</param>
	/// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
	virtual void SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) = 0;

	/// <summary>
	/// Adds or overrides a property of the context associated with this logger instance.
	/// </summary>
	/// <param name="name">Name of the property</param>
	/// <param name="value">GUID</param>
	virtual void SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) = 0;

    /// <summary>
    /// Logs the custom event with the specified name
    /// </summary>
    /// <param name="name">Name of the custom event</param>
    virtual void LogEvent(std::string const& name) = 0;

    /// <summary>
    /// Logs a custom event with specified name and properties to track information
    /// such as how a particular feature is used.
    /// </summary>
    /// <param name="properties">Properties of the custom event</param>
    virtual void LogEvent(EventProperties const& properties) = 0;
};


}}} // namespace Microsoft::Applications::Events 

#endif //ARIA_ILOGGER_H
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <mutex>

#include <opentelemetry/trace/key_value_iterable_view.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_id.h>
#include <opentelemetry/trace/trace_id.h>
#include <opentelemetry/trace/tracer_provider.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>
#include <cstdint>

#include "utils.hpp"

#include "LogManager.hpp"

//#pragma comment(lib, "ClientTelemetry.lib")
#pragma comment(lib, "WinInet.lib")

// Define it once per .exe or .dll in any compilation module
LOGMANAGER_INSTANCE

namespace core     = opentelemetry::core;
namespace trace    = opentelemetry::trace;

OPENTELEMETRY_BEGIN_NAMESPACE

/// <summary>
/// stream namespace provides no-exporter header-only implementation of local stream tracers:
/// - file
/// - ETW
/// - OutputDebugString
/// - console
///
/// </summary>
namespace oneds
{

static void CopyAttributesToProperties(const trace::KeyValueIterable &attributes, MAT::EventProperties &props)
{
  attributes.ForEachKeyValue([&](nostd::string_view key, common::AttributeValue value) noexcept {
    std::string name(key.data(), key.size());
    switch (value.index())
    {
      case common::AttributeType::TYPE_BOOL:
        props.SetProperty(name, std::get<bool>(value));
        break;
      case common::AttributeType::TYPE_INT:
        props.SetProperty(name, std::get<int32_t>(value));
        break;
      case common::AttributeType::TYPE_INT64:
        props.SetProperty(name, std::get<int64_t>(value));
        break;
      case common::AttributeType::TYPE_UINT:
        props.SetProperty(name, std::get<uint32_t>(value));
        break;
      case common::AttributeType::TYPE_UINT64:
        props.SetProperty(name, std::get<uint64_t>(value));
        break;
      case common::AttributeType::TYPE_DOUBLE:
        props.SetProperty(name, std::get<double>(value));
        break;
      case common::AttributeType::TYPE_STRING: {
        auto &sv = std::get<nostd::string_view>(value);
        props.SetProperty(name, std::string(sv.data(), sv.size()));
        break;
      }
      case common::AttributeType::TYPE_CSTRING:
        props.SetProperty(name, std::get<const char *>(value));
        break;
      case common::AttributeType::TYPE_SPAN_BYTE: {
        auto s = std::get<nostd::span<const uint8_t>>(value);
        // Unfortunately our 1DS vectors are int64_t only
        std::vector<int64_t> data;
        data.reserve(s.size());
        // FIXME: I have doubts about how performance-efficient this is..
        for (auto b : s)
          data.push_back(b);
        break;
        props.SetProperty(name, data);
      };
      default:
        /* TODO: unsupported type - add all other types here */
        break;
    }
    return true;
  });
};

class Span;

/// <summary>
/// stream::Tracer class that allows to send spans to stream
/// </summary>
class Tracer : public trace::Tracer
{
  /// <summary>
  /// Parent provider of this Tracer
  /// </summary>
  trace::TracerProvider &provider;

  MAT::ILogger* logger;

public:

  /// <summary>
  /// Tracer constructor
  /// </summary>
  /// <param name="parent">Parent TraceProvider</param>
  /// <param name="streamType">Stream type</param>
  /// <param name="arg2">Optional 2nd argument, e.g. filename</param>
  /// <returns>Tracer instance</returns>
  Tracer(trace::TracerProvider &parent, MAT::ILogger* logger)
      : trace::Tracer(),
        provider(parent),
        logger(logger)
  {

  }

  /// <summary>
  /// Start tracing Span
  /// </summary>
  /// <param name="name">Span name</param>
  /// <param name="options">Span options</param>
  /// <returns>Span</returns>
  virtual nostd::unique_ptr<trace::Span> StartSpan(
      nostd::string_view name,
      const trace::StartSpanOptions &options = {}) noexcept override
  {
    return trace::to_span_ptr<Span, Tracer>(this, name, options);
  }

  /// <summary>
  /// Force flush data to Tracer, spending up to given amount of microseconds to flush.
  /// </summary>
  /// <param name="timeout">Allow Tracer to drop data if timeout is reached</param>
  /// <returns>void</returns>
  virtual void ForceFlushWithMicroseconds(uint64_t timeout) noexcept override
  {
    MAT::LogManager::Flush();
  }

  /// <summary>
  /// Close tracer, spending up to given amount of microseconds to flush and close.
  /// </summary>
  /// <param name="timeout">Allow Tracer to drop data if timeout is reached</param>
  /// <returns></returns>
  virtual void CloseWithMicroseconds(uint64_t timeout) noexcept override
  {
    MAT::LogManager::Flush();
  }

  /// <summary>
  /// Add event data to span associated with tracer
  /// </summary>
  /// <param name="span"></param>
  /// <param name="name"></param>
  /// <param name="timestamp"></param>
  /// <param name="attributes"></param>
  /// <returns></returns>
  void AddEvent(Span &span, nostd::string_view name,
                core::SystemTimestamp timestamp,
                const trace::KeyValueIterable &attributes) noexcept
  {
    MAT::EventProperties props(std::string(name.data(), name.size()));
    CopyAttributesToProperties(attributes, props);
    logger->LogEvent(props);
  }

  /// <summary>
  /// Add event data to span associated with tracer
  /// </summary>
  /// <param name="span"></param>
  /// <param name="name"></param>
  /// <param name="timestamp"></param>
  /// <returns></returns>
  void AddEvent(Span &span, nostd::string_view name, core::SystemTimestamp timestamp) noexcept
  {
    logger->LogEvent(std::string(name.data(), name.size()));
    // TODO: set timestamp on event to timestamp
  }

  /// <summary>
  /// Add event data to span associated with tracer
  /// </summary>
  /// <param name="span"></param>
  /// <param name="name"></param>
  void AddEvent(Span &span, nostd::string_view name)
  {
    logger->LogEvent(std::string(name.data(), name.size()));
  }

};

/// <summary>
/// stream::Span allows to send event data to stream
/// </summary>
class Span : public trace::Span
{

protected:

  /// <summary>
  /// Parent (Owner) Tracer of this Span
  /// </summary>
  Tracer &owner;

public:

  /// <summary>
  /// Span constructor
  /// </summary>
  /// <param name="owner">Owner Tracer</param>
  /// <param name="name">Span name</param>
  /// <param name="options">Span options</param>
  /// <returns>Span</returns>
  Span(Tracer &owner, nostd::string_view name, const trace::StartSpanOptions &options) noexcept
      : trace::Span(), owner(owner)
  {
    (void)options;
  }

  ~Span()
  {
      End();
  }

  /// <summary>
  /// Add named event with no attributes
  /// </summary>
  /// <param name="name"></param>
  /// <returns></returns>
  void AddEvent(nostd::string_view name) noexcept
  {
      owner.AddEvent(*this, name);
  }

  /// <summary>
  /// Add named event with custom timestamp
  /// </summary>
  /// <param name="name"></param>
  /// <param name="timestamp"></param>
  /// <returns></returns>
  void AddEvent(nostd::string_view name, core::SystemTimestamp timestamp) noexcept
  {
    owner.AddEvent(*this, name, timestamp);
  }

  /// <summary>
  /// Add named event with custom timestamp and attributes
  /// </summary>
  /// <param name="name"></param>
  /// <param name="timestamp"></param>
  /// <param name="attributes"></param>
  /// <returns></returns>
  void AddEvent(nostd::string_view name,
                core::SystemTimestamp timestamp,
                const trace::KeyValueIterable &attributes) noexcept
  {
    owner.AddEvent(*this, name, timestamp, attributes);
  }

  /// <summary>
  /// Set Span status
  /// </summary>
  /// <param name="code"></param>
  /// <param name="description"></param>
  /// <returns></returns>
  void SetStatus(trace::CanonicalCode code, nostd::string_view description) noexcept
  {
      // TODO: not implemented
  }

  /// <summary>
  /// Update Span name
  /// </summary>
  /// <param name="name"></param>
  /// <returns></returns>
  void UpdateName(nostd::string_view name) noexcept
  {
      // TODO: not implemented
  }

  /// <summary>
  /// End Span
  /// </summary>
  /// <returns></returns>
  void End() noexcept
  {
      // TODO: signal this to owner
  }

  /// <summary>
  /// Check if Span is recording data
  /// </summary>
  /// <returns></returns>
  bool IsRecording() const noexcept
  {
      // TODO: not implemented
      return true;
  }

  /// <summary>
  /// Get Owner tracer of this Span
  /// </summary>
  /// <returns></returns>
  trace::Tracer &tracer() const noexcept
  {
      return this->owner;
  };

};

/// <summary>
/// stream::TraceProvider
/// </summary>
class TracerProvider : public trace::TracerProvider
{
public:

    struct LogManagerInitializer
    {
      LogManagerInitializer(std::string jsonConfig)
      {
          auto &config = MAT::LogManager::GetLogConfiguration();
          config       = MAT::FromJSON(jsonConfig.c_str());
          MAT::LogManager::Initialize();
      }
    };

  /// <summary>
  /// Obtain a Tracer of given type (name) and supply extra argument arg2 to it.
  /// </summary>
  /// <param name="name">Tracer Type</param>
  /// <param name="args">Tracer arguments</param>
  /// <returns></returns>
  virtual nostd::shared_ptr<trace::Tracer> GetTracer(nostd::string_view name,
                                                     nostd::string_view args = "")
  {
    // TODO: it is more approperiate to introduce a separate configuration API for
    // TracerProvider rather than using the 2nd argument to pass config here.
    static TracerProvider::LogManagerInitializer instance(std::string(args.data(), args.size()));
    auto logger = MAT::LogManager::GetLogger(std::string(name.data(), name.size()), "tracer");
    return nostd::shared_ptr<trace::Tracer>{new (std::nothrow) Tracer(*this, logger)};
  }

};

}  // namespace oneds
OPENTELEMETRY_END_NAMESPACE

//
// Example that illustrates the following concepts:
// - how to use ILogger-style event API
// - how to attach event::Properties object to span
// - how to implement a custom Tracer
//

#include <opentelemetry/event/Properties.hpp>

#include <opentelemetry/trace/key_value_iterable_view.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_id.h>
#include <opentelemetry/trace/trace_id.h>
#include <opentelemetry/trace/tracer_provider.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <map>
#include <string_view>

#include "Tracer.hpp"

#include "Config.hpp"

using namespace OPENTELEMETRY_NAMESPACE;

namespace trace = opentelemetry::trace;

using M = std::map<std::string_view, std::string_view>;

void test_spans()
{
    oneds::TracerProvider tp;
    auto tracer = tp.GetTracer("6d084bbf6a9644ef83f40a77c9e34580", CONFIG_ETW);
    auto span   = tracer->StartSpan("MySpan");

    // add m1 to span 1
    M m1 = {{"key1", "one"}, {"key2", "two"}};
    span->AddEvent("MyProduct.MyEvent1", m1);

    // add m2 to span 2
    M m2 = {{"key1", "one"}, {"key2", "two"}};
    span->AddEvent("MyProduct.MyEvent2", m2);

    // add map to span using initializer_list
    span->AddEvent("MyProduct.MyEvent3", {{"key1", "one"}, {"key2", "two"}});

    // Transform from EventProperties to collection of variant (AttributeValue)
    event::Properties myEvent(
        "MyProduct.MyEvent4",
        {/* C-string */ {"key1", "value1"},
         /* int32_t  */ {"intKey", 12345},
         /* bool     */ {"boolKey", static_cast<bool>(true)},
         /* GUID     */ {"guidKey1", event::UUID("00010203-0405-0607-0809-0A0B0C0D0E0F")}
        });
    auto name = myEvent.GetName();
    span->AddEvent(nostd::string_view(name.c_str(), name.length()), myEvent);

    span->End();
    // end tracing session
    tracer->Close();
}

int main(int argc, char *argv[])
{
  test_spans();
  return 0;
}

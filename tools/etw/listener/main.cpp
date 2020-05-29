// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <iostream>

#include "krabs.hpp"

#include <string>
#include <conio.h>
#include <thread>

std::wstring providerGuid;

krabs::user_trace trace;

void set_provider_guid(std::string guid)
{
    providerGuid.assign(guid.begin(), guid.end());
}

void user_trace_start()
{
    // A trace can have any number of providers, which are identified by GUID. These
    // GUIDs are defined by the components that emit events, and their GUIDs can
    // usually be found with various ETW tools (like wevutil).
    krabs::provider<> provider(krabs::guid(providerGuid.c_str()));

    // user_trace providers typically have any and all flags, whose meanings are
    // unique to the specific providers that are being invoked. To understand these
    // flags, you'll need to look to the ETW event producer.
    provider.any(0xffffffffffffffff);

    // providers should be wired up with functions (or functors) that are called when
    // events from that provider are fired.
    provider.add_on_event_callback([](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        // Once an event is received, if we want krabs to help us analyze it, we need
        // to snap in a schema to ask it for information.
        krabs::schema schema(record, trace_context.schema_locator);

        // We then have the ability to ask a few questions of the event.
        std::wcout << L"Event " << schema.event_id();
        std::wcout << L"(" << schema.event_name() << L") received." << std::endl;

        if (schema.event_id() == 7937)
        {
            // The event we're interested in has a field that contains a bunch of
            // info about what it's doing. We can snap in a parser to help us get
            // the property information out.
            krabs::parser parser(schema);

            // We have to explicitly name the type that we're parsing in a template
            // argument.
            // We could alternatively use try_parse if we didn't want an exception to
            // be thrown in the case of failure.
            std::wstring context = parser.parse<std::wstring>(L"ContextInfo");
            std::wcout << L"\tContext: " << context << std::endl;
        }
    });

    // the user_trace needs to know about the provider that we've set up.
    trace.enable(provider);
    static std::thread t([&] {
        printf("Starting trace...\n");
        trace.start();
        printf("Tracing stopped.\n");
    });
    // This is not ideal, but in most cases we are up and running by now
    t.detach();
    printf("Tracing thread detached.\n");
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    // "{780dddc8-18a1-5781-895a-a690464fa89c}");
    // "{56DC463B-97E8-4B59-E836-AB7C9BB96301}");
    set_provider_guid("{6D084BBF-6A96-44EF-83F4-0A77C9E34580}");

    // "{6D084BBF-6A96-44EF-83F4-0A77C9E34580}");  // "{4BB4D6F7-CAFC-4E92-92F9-72DCA2DCDE42}"); //"{780dddc8-18a1-5781-895a-a690464fa89c}" /* "{4BB4D6F7-CAFC-4E92-92F9-72DCA2DCDE42}" */);
    user_trace_start();

    printf("Press any key to stop tracing...\n");
    _getch();
    trace.stop();

    return 0;
}

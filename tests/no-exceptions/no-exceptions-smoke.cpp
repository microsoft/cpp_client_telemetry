//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "LogConfiguration.hpp"
#include "CorrelationVector.hpp"
#include "TransmitProfiles.hpp"
#include "http/HttpResponseDecoder.hpp"
#include "utils/ZlibUtils.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>

#if HAVE_EXCEPTIONS
#error This regression executable must be compiled without C++ exceptions.
#endif
#if !defined(HAVE_MAT_JSONHPP) || !defined(HAVE_MAT_ZLIB)
#error This regression executable requires JSON and zlib support.
#endif

namespace clienttelemetry {
    namespace data {
        namespace v3 {
            bool Expand(const char*, size_t, char**, size_t&, bool);
        }
    }
}

using namespace MAT;

namespace
{
    void check(bool condition, const char* message)
    {
        if (!condition)
        {
            std::fprintf(stderr, "FAILED: %s\n", message);
            std::exit(EXIT_FAILURE);
        }
    }

    [[noreturn]] void unexpectedSystemCall()
    {
        std::fputs("FAILED: response parsing unexpectedly accessed a telemetry service\n", stderr);
        std::exit(EXIT_FAILURE);
    }

    class UnusedSystem : public ITelemetrySystem
    {
    public:
        void start() override { unexpectedSystemCall(); }
        void stop() override { unexpectedSystemCall(); }
        void pause() override { unexpectedSystemCall(); }
        void resume() override { unexpectedSystemCall(); }
        bool upload() override { unexpectedSystemCall(); }
        void cleanup() override { unexpectedSystemCall(); }
        ILogManager& getLogManager() override { unexpectedSystemCall(); }
        IRuntimeConfig& getConfig() override { unexpectedSystemCall(); }
        ISemanticContext& getContext() override { unexpectedSystemCall(); }
        EventsUploadContextPtr createEventsUploadContext() override { unexpectedSystemCall(); }
        bool DispatchEvent(DebugEvent) override { unexpectedSystemCall(); }
        void sendEvent(IncomingEventContextPtr const&) override { unexpectedSystemCall(); }
        void handleFlushTaskDispatcher() override { unexpectedSystemCall(); }
        void signalDone() override { unexpectedSystemCall(); }
        void handleIncomingEventPrepared(IncomingEventContextPtr const&) override { unexpectedSystemCall(); }
        void preparedIncomingEventAsync(IncomingEventContextPtr const&) override { unexpectedSystemCall(); }
    };

    class ResponseDecoder : public HttpResponseDecoder
    {
    public:
        explicit ResponseDecoder(ITelemetrySystem& system) : HttpResponseDecoder(system) {}
        using HttpResponseDecoder::processBody;
        unsigned expiredTickets = 0;
        bool DispatchEvent(DebugEvent event) override
        {
            check(event.type == DebugEventType::EVT_TICKET_EXPIRED, "unexpected response event");
            ++expiredTickets;
            return true;
        }
    };

    class ProfileAccess : public TransmitProfiles
    {
    public:
        static const std::map<std::string, TransmitProfileRules>& loaded() { return profiles; }
    };
}

int main()
{
    const char* invalidConfigurations[] = {nullptr, "", "[", "not-json", "null", "1", "\"text\"", "[]", "[{}]"};
    for (const char* input : invalidConfigurations)
    {
        auto config = FromJSON(input);
        check((*config).empty(), "invalid configuration must return empty without aborting");
    }
    auto config = FromJSON(R"({"enabled":true,"signed":-9223372036854775808,"unsigned":18446744073709551615,"nested":{"key":"value"}})");
    check(config.HasConfig("enabled") && config.HasConfig("nested"), "valid configuration was lost");
    check(static_cast<int64_t>(config["signed"]) == std::numeric_limits<int64_t>::min(), "signed configuration limit");
    check(static_cast<uint64_t>(config["unsigned"]) == std::numeric_limits<uint64_t>::max(), "unsigned configuration limit");

    for (const char* input : {
        "not-json", "[", "[null]", R"([{"name":1}])",
        R"([{"name":"Bad","rules":[{"timers":[2147483648]}]}])",
        R"([{"name":"Bad","rules":[{"timers":[-2147483649]}]}])",
        R"([{"name":"Bad","rules":[{"timers":[1e999]}]}])"})
    {
        check(!TransmitProfiles::load(input), "malformed profiles must return false without aborting");
    }
    check(TransmitProfiles::load(R"([{"name":"Limits","rules":[{"timers":[-2147483648,2147483647,1.5]}]}])"), "valid timer limits");
    check(ProfileAccess::loaded().at("Limits").rules[0].timers ==
          std::vector<int>({std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 1}), "timer limit conversion");
    check(TransmitProfiles::load(R"([{"name":"Prefix","rules":[]},{"name":null},{"name":"Skipped"}])"), "partial profile loading");
    check(ProfileAccess::loaded().count("Prefix") == 1 &&
          ProfileAccess::loaded().count("Skipped") == 0 &&
          ProfileAccess::loaded().count("Limits") == 0, "partial loading must replace only the valid prefix");

    UnusedSystem system;
    ResponseDecoder decoder(system);
    SimpleHttpResponse response("no-exceptions");
    for (const char* input : {
        "not-json", "[", "null", "1", "[]",
        R"({"efi":null})", R"({"efi":1})", R"({"efi":"all"})", R"({"efi":["all"]})",
        R"({"acc":1e100,"efi":{"tenant":"all"}})",
        R"({"rej":18446744073709551615,"efi":{"tenant":"all"}})"})
    {
        const std::string body(input);
        response.m_body.assign(body.begin(), body.end());
        auto result = Accepted;
        decoder.processBody(response, result);
        check(result == Accepted && decoder.expiredTickets == 0, "malformed response changed upload outcome");
    }
    const std::string validResponse = R"({"acc":2147483647,"rej":0,"efi":{"tenant":"all"},"TokenCrackingFailure":true})";
    response.m_body.assign(validResponse.begin(), validResponse.end());
    auto result = Accepted;
    decoder.processBody(response, result);
    check(result == Rejected && decoder.expiredTickets == 1, "valid response lost rejection or ticket event");

    CorrelationVector vector;
    check(vector.SetValue("jj9XLhDw7EuXoC2L.4294967295"), "maximum correlation vector element");
    check(!vector.SetValue("jj9XLhDw7EuXoC2L.4294967296"), "overflowing correlation vector element");
    check(!vector.SetValue("jj9XLhDw7EuXoC2L.9999999999999999999999999"), "oversized correlation vector element");

    const char invalidCompressed[] = "invalid";
    char* output = nullptr;
    size_t outputSize = std::numeric_limits<size_t>::max();
    check(!clienttelemetry::data::v3::Expand(invalidCompressed, sizeof(invalidCompressed), &output, outputSize, false),
          "oversized expansion must fail");
    check(output == nullptr && outputSize == 0, "failed expansion must clear outputs");
    std::puts("Passed no-exceptions configuration, response, profile, numeric-limit, and expansion smoke checks.");
    return EXIT_SUCCESS;
}

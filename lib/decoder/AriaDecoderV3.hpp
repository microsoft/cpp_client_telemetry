#ifndef AriaDecoderV3_HPP
#define AriaDecoderV3_HPP

#include "mat/config.h"

#include <vector>
#include <cinttypes>
#include <iostream>

#ifdef __cplusplus_cli
#else
#include "zlib.h"
#undef compress
#endif

#ifdef _WIN32
#include <Windows.h>
// XXX: sometimes Windows.h defines max as a macro in some Windows SDKs
#ifdef max
#undef max
#undef min
#endif
#endif

/* Include json.hpp if available */
#if defined(__has_include) && __has_include ("json.hpp")
#ifndef HAVE_MAT_JSONHPP
#define HAVE_MAT_JSONHPP
#endif
#endif

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

#include "bond/All.hpp"
#include "bond/generated/CsProtocol_types.hpp"
#include "bond/generated/CsProtocol_readers.hpp"

class AriaDecoderV3
{
public:

#ifdef HAVE_MAT_JSONHPP
    static void to_json(nlohmann::json& j, const CsProtocol::Record& r);
#endif

    static void decode(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool compressed = true);

    static void ExpandVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out);
    static void InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out);
};

#endif

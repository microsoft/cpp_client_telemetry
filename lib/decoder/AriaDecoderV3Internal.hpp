#ifndef AriaDecoderV3Internal_HPP
#define AriaDecoderV3Internal_HPP

#include "AriaDecoderV3.hpp"

#include "mat/config.h"

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

namespace AriaDecoderV3 {

#ifdef HAVE_MAT_JSONHPP
    void to_json(nlohmann::json& j, const CsProtocol::Record& r);
#endif

}  // namespace AriaDecoderV3

#endif

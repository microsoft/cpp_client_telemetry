#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#include "pal/PAL.hpp"

#include <string>

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    // enum return codes
    enum ECS_ERROR
    {
        ECS_ERROR_OK = 0,
        ECS_ERROR_INVALID_CONFIG,
        ECS_ERROR_INVALID_STATUS,
        ECS_ERROR_INVALID_PARAMETER,
        ECS_ERROR_INVALID_AUTHTOKEN,
        ECS_ERROR_INVALID_ECS_RESPONSE,
        ECS_ERROR_HTTPSTACK_INIT_FAILED,
        ECS_ERROR_LOAD_OFFLINESTORAGE_FAILED,
        ECS_ERROR_START_THREAD_FAILED,
        ECS_ERROR_OUT_OF_MEMORY,
        ECS_ERROR_UNKNOWN_FAILURE
    };

    std::string CreateServerUrl(
        const std::string& serverUrl,
        const std::string& clientName,
        const std::string& clientVersion);

	MATSDK_LOG_DECL_COMPONENT_NS();
}}}}
#endif

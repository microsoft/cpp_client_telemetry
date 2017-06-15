#pragma once

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

    void ThrowECSError(ECS_ERROR errCode, const char* pFile, int lineNumber);

    std::string GetStoragePath();

// Exceptions defines
#define THROW			        throw
#define CONST_THROW		        const throw()
#define BOND_THROW(x, y)	    throw x((bond::detail::string_stream() << y).content());
#define ECS_THROW(errCode, ...) \
        do\
        {\
        ThrowECSError(errCode, __FILE__, __LINE__);\
        } while(0,0)

}}}}

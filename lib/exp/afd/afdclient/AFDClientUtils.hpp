#pragma once

#include <string>
#include <vector>

namespace Microsoft { namespace Applications { namespace Experimentation { namespace AFD {

    // enum return codes
    enum AFD_ERROR
    {
        AFD_ERROR_OK = 0,
        AFD_ERROR_INVALID_CONFIG,
        AFD_ERROR_INVALID_STATUS,
        AFD_ERROR_INVALID_PARAMETER,
        AFD_ERROR_INVALID_AUTHTOKEN,
        AFD_ERROR_INVALID_AFD_RESPONSE,
        AFD_ERROR_HTTPSTACK_INIT_FAILED,
        AFD_ERROR_LOAD_OFFLINESTORAGE_FAILED,
        AFD_ERROR_START_THREAD_FAILED,
        AFD_ERROR_OUT_OF_MEMORY,
        AFD_ERROR_UNKNOWN_FAILURE
    };

    std::string CreateServerUrl(
        const std::string& serverUrl,
        const std::string& clientName);

    void ThrowAFDError(AFD_ERROR errCode, const char* pFile, int lineNumber);

    std::string GetStoragePath();
    // You could also take an existing vector as a parameter.
    std::vector<std::string> splitString(std::string str, char delimiter);
    

// Exceptions defines
#define THROW			        throw
#define CONST_THROW		        const throw()
#define BOND_THROW(x, y)	    throw x((bond::detail::string_stream() << y).content());
#define AFD_THROW(errCode, ...) \
        do\
        {\
        ThrowAFDError(errCode, __FILE__, __LINE__);\
        } while(0,0)

}}}}

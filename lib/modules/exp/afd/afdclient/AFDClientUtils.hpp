#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#pragma once

#include "pal/PAL.hpp"

#include <string>
#include <vector>

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace AFD
            {

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

                // You could also take an existing vector as a parameter.
                std::vector<std::string> splitString(const std::string& str, char delimiter);

                MATSDK_LOG_DECL_COMPONENT_NS();

            }
        }
    }
}
#endif

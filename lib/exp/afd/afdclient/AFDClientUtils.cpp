#define LOG_MODULE DBG_API

#include "AFDClientUtils.hpp"
#include "pal/PAL.hpp"
#include <assert.h>
#include <string>
#include <iostream>
#include <string>
#include <sstream>

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace AFD {

                ARIASDK_LOG_INST_COMPONENT_NS("EventsSDK.AFD", "Aria AFD");

                std::vector<std::string> splitString(const std::string& str, char delimiter)
                {
                    std::vector<std::string> internal;
                    std::stringstream ss(str);
                    std::string token;

                    while (getline(ss, token, delimiter))
                    {
                        internal.push_back(token);
                    }

                    return internal;
                }
            }
        }
    }
}
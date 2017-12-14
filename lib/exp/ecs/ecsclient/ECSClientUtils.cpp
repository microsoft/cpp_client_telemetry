#define LOG_MODULE DBG_API

#include "ECSClientUtils.hpp"
#include "pal/PAL.hpp"
#include "../../JsonHelper.hpp"
#include <assert.h>
#include <string>
#include <stdexcept>


namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace ECS {

				ARIASDK_LOG_INST_COMPONENT_NS("EventsSDK.ECS", "Aria ECS");

				// replace "/" in clientVersion with "_"
				std::string ConvertClientVersion(const std::string& clientVersion)
				{
					std::string clientVer;

					if (!clientVersion.empty())
					{
						for (const char* p = clientVersion.c_str(); *p; ++p)
						{
							clientVer += (*p == '/' ? '_' : *p);
						}
					}

					return clientVer;
				}

                std::string CreateServerUrl(
                    const std::string& serverUrl,
                    const std::string& clientName,
                    const std::string& clientVersion)
                {
                    assert(!serverUrl.empty());
                    assert(!clientName.empty());

                    std::string resultUrl = JsonHelper::Combine(serverUrl, clientName, '/');

                    std::string clientVer = ConvertClientVersion(clientVersion);
                    if (!clientVer.empty())
                    {
                        resultUrl = JsonHelper::Combine(resultUrl, clientVer, '/');
                    }

                    return resultUrl;
                }

            }
        }
    }
}
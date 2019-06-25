#include "Version.hpp"
#include "Contexts.hpp"
#include "CommonFields.h"
#include <vector>
#include <string>

namespace ARIASDK_NS_BEGIN
{
    const char* const ARIA_EXT_TEXT = "ext";
    const char* const ARIA_DATA_TEXT = "data";
    const char* const ARIA_EXT_EXTAPP_TEXT = "extApp";
    const char* const ARIA_EXT_EXTOS_TEXT = "os";
    const char* const ARIA_EXT_EXTNET_TEXT = "extNet";
    const char* const ARIA_EXT_EXTMETADATA_TEXT = "metadata";

    class MATSDK_LIBABI JsonFormatter
    {
    public:
        JsonFormatter();

        ~JsonFormatter();

        std::string getJsonFormattedEvent(IncomingEventContextPtr const& event);
    };

} ARIASDK_NS_END
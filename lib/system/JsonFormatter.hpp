#include "Version.hpp"
#include "Contexts.hpp"
#include "CommonFields.h"
#include <vector>
#include <string>

namespace MAT_NS_BEGIN
{
    class MATSDK_LIBABI JsonFormatter
    {
    public:
        JsonFormatter();

        ~JsonFormatter();

        std::string getJsonFormattedEvent(IncomingEventContextPtr const& event);
    };

} MAT_NS_END
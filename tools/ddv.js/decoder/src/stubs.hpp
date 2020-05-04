// Temporary stub for now. We don't need this C API function because we're not linking
// the SDK in decoder, but the default mat/config.h requires it. TODO: get rid of it
#include <mat.h>

EVTSDK_LIBABI evt_status_t EVTSDK_LIBABI_CDECL evt_api_call_default(evt_context_t*)
{
    return (evt_status_t)0;
}
// end of temporary stub

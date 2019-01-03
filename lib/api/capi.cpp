#ifdef _WIN32
#define ARIASDK_DECLSPEC __declspec(dllexport)
#endif

#include "LogManagerProvider.hpp"
#include "aria.h"
#include "utils/Utils.hpp"
#include <map>

using namespace MAT;

typedef struct
{
    ILogConfiguration cfg;
    ILogManager * lm;
} entry;

// FIXME: [MG] - add locking around lms
std::map < evt_handle_t, entry > lms;

/**
 * Obtain log manager ptr using C API handle
 */
ILogManager* GetLogManager(evt_handle_t handle)
{
    auto it = lms.find(handle);
    return (it != lms.end()) ?
        it->second.lm :
        nullptr;
}

evt_status_t aria_open(evt_context_t *ctx)
{
    char* config = (char *)ctx->data;

    evt_handle_t code = static_cast<evt_handle_t>(hashCode(config));
    auto it = lms.find(code);
    if (it != lms.end())
    {
        // Already open?..
        //
        // FIXME: [MG] - check for hashCode collisions. If there's a collision - different tenant,
        // but same hash code, then keep trying with a different seed until a good unused hash is
        // found.
        return EALREADY;
    }

    if (config == nullptr)
    {
        // Invalid configuration
        return EFAULT;
    }

    if (config[0] == '{')
    {
        // Create new configuration object from JSON
        lms[code].cfg = MAT::FromJSON(config);
    }
    else
    {
        // Assume that config is a token string
        std::string moduleName = "CAPI-Client-";
        moduleName += std::to_string(code);
        lms[code].cfg =
        {
            { "name", moduleName },
            { "version", "1.0.0" },
            { "config",
                {
                    { "host", "*" }
                }
            },
            { CFG_STR_PRIMARY_TOKEN, config }
        };
    }

    // Pass a reference to obtain or create new log manager instance
    auto & cfg = lms[code].cfg;
    status_t status;
    ILogManager *lm = LogManagerProvider::CreateLogManager(cfg, status);

    // Verify that the instance pointer is valid
    if (lm == nullptr)
    {
        status = static_cast<status_t>(EFAULT);
    }
    else
    {
        // Remember the pointer to current ILogManager instance
        lms[code].lm = lm;
    }

    ctx->result = (evt_status_t)status;
    ctx->handle = code;
    return ctx->result;
}

//
// Marashal C struct tro Aria C++ API
//
evt_status_t aria_log(evt_context_t *ctx)
{
    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }

    evt_prop *evt = (evt_prop*)ctx->data;
    EventProperties props;
    props.unpack(evt);

    // TODO: should we remove the iKey from props?
    auto m = props.GetProperties();
    EventProperty &prop = m["iKey"];
    std::string token = prop.as_string;

    // TODO: should we support source passed in evt?
    ILogger *logger = lm->GetLogger(token);
    logger->LogEvent(props);

    ctx->result = EOK;
    return EOK;
}

evt_status_t aria_close(evt_context_t *ctx)
{
    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }
    auto & cfg = lm->GetLogConfiguration();
    return (evt_status_t)LogManagerProvider::Release(cfg);
}

evt_status_t aria_pause(evt_context_t *ctx)
{
    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }
    return (evt_status_t)lm->PauseTransmission();
}

evt_status_t aria_resume(evt_context_t *ctx)
{
    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }
    return (evt_status_t)lm->ResumeTransmission();
}

evt_status_t aria_upload(evt_context_t *ctx)
{

    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }
    return (evt_status_t)lm->UploadNow();
}

evt_status_t aria_flush(evt_context_t *ctx)
{
    ILogManager *lm = GetLogManager(ctx->handle);
    if (lm == nullptr)
    {
        return ENOENT;
    }
    return (evt_status_t)lm->Flush();
}

extern "C" {

    /**
     * Simple stable backwards- / forward- compatible ABI interface
     */
    evt_status_t EVTSDK_LIBABI_CDECL evt_api_call_default(evt_context_t *ctx)
    {
        evt_status_t result = EFAIL;

        if (ctx != nullptr)
        {
            switch (ctx->call)
            {
            case EVT_OP_LOAD:
                result = ENOTSUP;
                break;

            case EVT_OP_UNLOAD:
                result = ENOTSUP;
                break;

            case EVT_OP_OPEN:
                result = aria_open(ctx);
                break;

            case EVT_OP_CLOSE:
                result = aria_close(ctx);
                break;

            case EVT_OP_CONFIG:
                result = ENOTSUP;
                break;

            case EVT_OP_LOG:
                result = aria_log(ctx);
                break;

            case EVT_OP_PAUSE:
                result = aria_pause(ctx);
                break;

            case EVT_OP_RESUME:
                result = aria_resume(ctx);
                break;

            case EVT_OP_UPLOAD:
                result = aria_upload(ctx);
                break;

            case EVT_OP_FLUSH:
                result = aria_flush(ctx);
                break;

            case EVT_OP_VERSION:
                result = ENOTSUP;
                break;

            // Add more OPs here

            default:
                result = ENOTSUP;
                break;
            }
        }
        return result;
    }
}

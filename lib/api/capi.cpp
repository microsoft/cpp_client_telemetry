// Copyright (c) Microsoft Corporation. All rights reserved.

#include "mat/config.h"

#if !defined (ANDROID) || defined(ENABLE_CAPI_HTTP_CLIENT)
#include "http/HttpClient_CAPI.hpp"
#endif
#include "LogManagerProvider.hpp"
#include "mat.h"
#include "pal/TaskDispatcher_CAPI.hpp"
#include "utils/Utils.hpp"

#include "LogManager.hpp"

#include "pal/PAL.hpp"

#include "CommonFields.h"
#include "config/RuntimeConfig_Default.hpp"

#include <mutex>
#include <map>
#include <cstdint>
#include <memory>

static const char * libSemver = TELEMETRY_EVENTS_VERSION;

using namespace MAT;

static std::mutex mtx;
static std::map<evt_handle_t, capi_client> clients;

/// <summary>
/// Convert from C API handle to internal C API client struct.
///
/// This method may be used for C API flow debugging, i.e. to obtain the
/// underlying instance of ILogManager and attach a debug event callback.
///
/// NOTE: method is not guaranteed to be ABI-stable and should not be used
/// across dynamic / shared library boundary. Underlying ILogManager /
/// ILogConfiguration are implemented in C++ and do not provide ABI-stable
/// guarantee from one SDK version to another.
/// </summary>
capi_client * MAT::capi_get_client(evt_handle_t handle)
{
    LOCKGUARD(mtx);
    const auto it = clients.find(handle);
    return (it != clients.cend()) ? &(it->second) : nullptr;
}

/// <summary>
/// Remove C API handle from active client tracking struct.
/// </summary>
void remove_client(evt_handle_t handle)
{
    LOCKGUARD(mtx);
    clients.erase(handle);
}

#define VERIFY_CLIENT_HANDLE(client, ctx)                       \
    if (ctx==nullptr)                                           \
    {                                                           \
        return EFAULT; /* bad address */                        \
    };                                                          \
    auto client = MAT::capi_get_client(ctx->handle);            \
    if ((client == nullptr) || (client->logmanager == nullptr)) \
    {                                                           \
        return ENOENT;                                          \
    };

evt_status_t mat_open_core(
    evt_context_t *ctx,
    const char* config,
    http_send_fn_t httpSendFn,
    http_cancel_fn_t httpCancelFn,
    task_dispatcher_queue_fn_t taskDispatcherQueueFn,
    task_dispatcher_cancel_fn_t taskDispatcherCancelFn,
    task_dispatcher_join_fn_t taskDispatcherJoinFn)
{
    if ((config == nullptr) || (config[0] == 0))
    {
        // Invalid configuration
        return EFAULT;
    }

    evt_handle_t code = static_cast<evt_handle_t>(hashCode(config));
    bool isHashFound = false;
    // Find the next available spare hashcode
    do
    {
        const auto client = MAT::capi_get_client(code);
        if (client != nullptr)
        {
            if (client->ctx_data == config)
            {
                // Guest or Host instance with the same config is already open
                ctx->handle = code;
                return EALREADY;
            }
            // hash code is assigned to another client, increment and retry
            // with the next empty slot
            code++;
            continue;
        };
        isHashFound = true;
    } while (!isHashFound);

    // Make sure that we fully inherit the default configuration, then
    // overlay custom configuration on top of default.
    clients[code].config = ILogConfiguration();
    Variant::merge_map(*clients[code].config, *defaultRuntimeConfig);

    // JSON configuration must start with {
    if (config[0] == '{')
    {
        // Create new configuration object from JSON
        ILogConfiguration jsonConfig = MAT::FromJSON(config);
        // Overwrite default values with custom configuration.
        Variant::merge_map(*clients[code].config, *jsonConfig, true);
    }
    else
    {
        // Assume that the config string is a token, not JSON.
        // That approach allows to consume the lightweght C API without JSON parser compiled in.
        std::string moduleName = "CAPI-Client-";
        moduleName += std::to_string(code);
        VariantMap customConfig =
        {
            { CFG_STR_FACTORY_NAME, moduleName },
            { "version", "1.0.0" },
            { CFG_MAP_FACTORY_CONFIG,
                {
                    { CFG_STR_FACTORY_HOST, "*" },
                    { CFG_STR_CONTEXT_SCOPE, CONTEXT_SCOPE_NONE }
                }
            },
            { CFG_STR_PRIMARY_TOKEN, config }
        };
        // Overwrite host-guest related settings using VariantMap above.
        Variant::merge_map(*clients[code].config, customConfig, true);
    }

    // Remember the original config string. Needed to avoid hash code collisions
    clients[code].ctx_data = config;

#if !defined (ANDROID) || defined(ENABLE_CAPI_HTTP_CLIENT)
    // Create custom HttpClient
    if (httpSendFn != nullptr && httpCancelFn != nullptr)
    {
        try
        {
            auto http = std::make_shared<HttpClient_CAPI>(httpSendFn, httpCancelFn);
            clients[code].http = http;
            clients[code].config.AddModule(CFG_MODULE_HTTP_CLIENT, http);
        }
        catch (...)
        {
            return EFAULT;
        }
    }
#endif
    // Create custom worker thread
    if (taskDispatcherQueueFn != nullptr && taskDispatcherCancelFn != nullptr && taskDispatcherJoinFn != nullptr)
    {
        try
        {
            auto taskDispatcher = std::make_shared<PAL::TaskDispatcher_CAPI>(taskDispatcherQueueFn, taskDispatcherCancelFn, taskDispatcherJoinFn);
            clients[code].taskDispatcher = taskDispatcher;
            clients[code].config.AddModule(CFG_MODULE_TASK_DISPATCHER, taskDispatcher);
        }
        catch (...)
        {
            return EFAULT;
        }
    }

    status_t status = static_cast<status_t>(EFAULT);
    clients[code].logmanager = LogManagerProvider::CreateLogManager(clients[code].config, status);

    // Verify that the instance pointer is valid
    if (clients[code].logmanager == nullptr)
    {
        status = static_cast<status_t>(EFAULT);
    }
    ctx->result = static_cast<evt_status_t>(status);
    ctx->handle = code;
    return ctx->result;
}

evt_status_t mat_open(evt_context_t *ctx)
{
    if (ctx == nullptr)
    {
        return EFAULT; /* bad address */
    };

    char* config = static_cast<char *>(ctx->data);
    return mat_open_core(ctx, config, nullptr, nullptr, nullptr, nullptr, nullptr);
}

evt_status_t mat_open_with_params(evt_context_t *ctx)
{
    if (ctx == nullptr)
    {
        return EFAULT; /* bad address */
    };

    const evt_open_with_params_data_t* data = static_cast<evt_open_with_params_data_t*>(ctx->data);
    if ((data == nullptr) || (data->params == nullptr))
    {
        // Invalid param data
        return EFAULT;
    }

    http_send_fn_t httpSendFn = nullptr;
    http_cancel_fn_t httpCancelFn = nullptr;
    task_dispatcher_queue_fn_t taskDispatcherQueueFn = nullptr;
    task_dispatcher_cancel_fn_t taskDispatcherCancelFn = nullptr;
    task_dispatcher_join_fn_t taskDispatcherJoinFn = nullptr;

    for (int32_t i = 0; i < data->paramsCount; ++i) {
        const evt_open_param_t& param = data->params[i];
        switch (param.type) {
            case OPEN_PARAM_TYPE_HTTP_HANDLER_SEND:
                httpSendFn = reinterpret_cast<http_send_fn_t>(param.data);
                break;
            case OPEN_PARAM_TYPE_HTTP_HANDLER_CANCEL:
                httpCancelFn = reinterpret_cast<http_cancel_fn_t>(param.data);
                break;
            case OPEN_PARAM_TYPE_TASK_DISPATCHER_QUEUE:
                taskDispatcherQueueFn = reinterpret_cast<task_dispatcher_queue_fn_t>(param.data);
                break;
            case OPEN_PARAM_TYPE_TASK_DISPATCHER_CANCEL:
                taskDispatcherCancelFn = reinterpret_cast<task_dispatcher_cancel_fn_t>(param.data);
                break;
            case OPEN_PARAM_TYPE_TASK_DISPATCHER_JOIN:
                taskDispatcherJoinFn = reinterpret_cast<task_dispatcher_join_fn_t>(param.data);
                break;
        }
    }

    return mat_open_core(ctx, data->config, httpSendFn, httpCancelFn, taskDispatcherQueueFn, taskDispatcherCancelFn, taskDispatcherJoinFn);
}

/**
 * Marashal C struct to C++ API for the following methods:
 * - ILogger->LogEvent(...)
 * - ILogger->SetContext(...) - for each string key - prop value
 * - ILogManager->SetContext(...) - for each string key - prop value
 */
evt_status_t mat_sendprops(evt_context_t* ctx, evt_call_t op)
{
    VERIFY_CLIENT_HANDLE(client, ctx);

    ILogConfiguration & config = client->config;
    evt_prop *evt = static_cast<evt_prop*>(ctx->data);
    EventProperties props;
    props.unpack(evt, ctx->size);

    // Determine ingestion token to use for this record.
    std::string token;

    // Use LogManager configuration primary token if available.
    if (config.HasConfig(CFG_STR_PRIMARY_TOKEN))
    {
        token = static_cast<const char*>(config[CFG_STR_PRIMARY_TOKEN]);
    }

    // Allow to override iKey per event via property.
    // C API client maintains one handle for different tenants.
    auto m = props.GetProperties();
    if (m.count(COMMONFIELDS_IKEY))
    {
        EventProperty& prop = m[COMMONFIELDS_IKEY];
        token = prop.as_string;
        props.erase(COMMONFIELDS_IKEY);
    }

    // Privacy feature for OTEL C API client:
    //
    // C API customer that does not explicitly pass down JSON
    //   config["config]["scope"] = COMMONFIELDS_SCOPE_ALL;
    //
    // should not be able to capture the host's context vars.
    std::string scope = CONTEXT_SCOPE_NONE;
    {
        MAT::VariantMap &config_map = config[CFG_MAP_FACTORY_CONFIG];
        const auto & it = config_map.find(CFG_STR_CONTEXT_SCOPE);
        if (it != config_map.cend())
        {
            scope = static_cast<const char *>(it->second);
            // Specifying "*" in JSON config allows Guest C API logger to capture Host context variables
            if (scope == CONTEXT_SCOPE_ALL)
            {
                scope = CONTEXT_SCOPE_EMPTY;
            }
        }
    }

    const auto & it = m.find(COMMONFIELDS_EVENT_SOURCE);
    std::string source = ((it != m.cend()) && (it->second.type == EventProperty::TYPE_STRING)) ? it->second.as_string : "";

    ILogger *logger = client->logmanager->GetLogger(token, source, scope);
    if (logger == nullptr)
    {
        ctx->result = EFAULT; /* invalid address */
        return ctx->result;
    }

    // Enforce if scope sharing is not enabled (default).
    // However, if a client / extension explicitly opts-in to append its
    // context to host variables, then we respect their choice and append
    // the corresponding host context values on guest events. This is
    // required for the case where the app, SDK, and extension SDKs run
    // within the same data silo / trust boundary, ensuring uniform and
    // consistent symmetric data contract for all parties involved.
    if (scope == CONTEXT_SCOPE_NONE)
    {
        if (op == EVT_OP_SET_LOGMANAGER_CONTEXT)
        {
            // We are running in isolation. It does not make sense to propagate
            // the context at LogManager (parent) -level since we do not have
            // ability to modify it. Downgrade to EVT_OP_SET_LOGGER_CONTEXT.
            op = EVT_OP_SET_LOGGER_CONTEXT;
        }
    }

    // Allows to pass C API properties to ILogger or LogManager semantic context.
    // Semantic context layer provides access to both - Common Fields (Part A),
    // as well as Custom Fields (Part C). For C API due to performance reasons -
    // the determination of what is considered Common versus Custom is done
    // by checking the first 4 bytes of context property name:
    // - This is in alignment with canonical Common Schema JSON notation.
    // - This is much faster than validating the list of old-style COMMONFIELDS_
    // aliases.
    //
    // Developers can find the mapping on 1DS site or use ContextFieldProvider.cpp
    // as a reference.
    auto populateContext = [m](ISemanticContext& context)
    {
        for (auto prop : m)
        {
            if (prop.first.rfind("ext.", 0) == 0)
            {
                context.SetCommonField(prop.first, prop.second);
            }
            else
            {
                context.SetCustomField(prop.first, prop.second);
            }
        }
    };

    switch (op)
    {
    // This path allows to implement common props stamping via C#->C->C++ API.
    // Calls ILogger->SetContext(...) to populate the "local" current ILogger context.
    case EVT_OP_SET_LOGGER_CONTEXT:
        {
            const auto loggerContext = logger->GetSemanticContext();
            populateContext(*loggerContext);
            ctx->result = EOK;
        }
        break;

    // This path allows to implement common props stamping via C#->C->C++ API.
    // Calls ILogManager->SetContext(...) to populate the "global" LogManager context.
    case EVT_OP_SET_LOGMANAGER_CONTEXT:
        {
            auto& logManagerContext = client->logmanager->GetSemanticContext();
            populateContext(logManagerContext);
            ctx->result = EOK;
        }
        break;

    // Original implementation of evt_log call remains unaltered. Note that the processing
    // of Common Fields via C API could only done using:
    // - set_logger_context(_s)
    // - set_logmanager_context(_s)
    // Practical reasons:
    // - done intentionally to avoid altering behavior of legacy code.
    // - Part A extension values typically remain constant for an app/user session.
    case EVT_OP_LOG:
        logger->LogEvent(props);
        ctx->result = EOK;
        break;

    default:
        // Unsupported API. Call failed.
        ctx->result = EFAULT;
        break;
    }
    return ctx->result;
}

evt_status_t mat_close(evt_context_t *ctx)
{
    VERIFY_CLIENT_HANDLE(client, ctx);
    const auto result = static_cast<evt_status_t>(LogManagerProvider::Release(client->logmanager->GetLogConfiguration()));
    
    if (client->http != nullptr)
    {
        client->http = nullptr;
    }

    if (client->taskDispatcher != nullptr)
    {
        client->taskDispatcher = nullptr;
    }

    remove_client(ctx->handle);
    ctx->result = result;
    return result;
}

evt_status_t mat_pause(evt_context_t *ctx)
{
    VERIFY_CLIENT_HANDLE(client, ctx);
    const auto result = static_cast<evt_status_t>(client->logmanager->PauseTransmission());
    ctx->result = result;
    return result;
}

evt_status_t mat_resume(evt_context_t *ctx)
{
    VERIFY_CLIENT_HANDLE(client, ctx);
    const auto result = static_cast<evt_status_t>(client->logmanager->ResumeTransmission());
    ctx->result = result;
    return result;
}

evt_status_t mat_upload(evt_context_t *ctx)
{
    VERIFY_CLIENT_HANDLE(client, ctx);
    const auto result = static_cast<evt_status_t>(client->logmanager->UploadNow());
    ctx->result = result;
    return result;
}

evt_status_t mat_flush(evt_context_t *ctx)
{
    VERIFY_CLIENT_HANDLE(client, ctx);
    const auto result = static_cast<evt_status_t>(client->logmanager->Flush());
    ctx->result = result;
    return result;
}

extern "C" {

    /**
     * Simple stable backwards- / forward- compatible ABI interface
     */
    MATSDK_LIBABI evt_status_t EVTSDK_LIBABI_CDECL evt_api_call_default(evt_context_t* ctx)
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
                result = mat_open(ctx);
                break;

            case EVT_OP_OPEN_WITH_PARAMS:
                result = mat_open_with_params(ctx);
                break;

            case EVT_OP_CLOSE:
                result = mat_close(ctx);
                break;

            case EVT_OP_CONFIG:
                result = ENOTSUP;
                break;

            case EVT_OP_LOG:
                result = mat_sendprops(ctx, EVT_OP_LOG);
                break;

            case EVT_OP_PAUSE:
                result = mat_pause(ctx);
                break;

            case EVT_OP_RESUME:
                result = mat_resume(ctx);
                break;

            case EVT_OP_UPLOAD:
                result = mat_upload(ctx);
                break;

            case EVT_OP_FLUSH:
                result = mat_flush(ctx);
                break;

            case EVT_OP_VERSION:
                // TODO: add handling of ctx->data passed by caller inline stub :
                // If there is API version mismatch between the stub and lib impl, then
                // depending on version passed down to SDK - lib may need to figure out
                // how to handle the mismatch. For now the onus of verifying for SDK
                // compatibility is on API caller.

                // Inlined stub version passed by the caller - compiled in executable.
                LOG_TRACE("header  version: %s", ctx->data);

                // Actual SDK library implementation version - compiled in the library.
                ctx->data = (void*)libSemver;
                LOG_TRACE("library version: %s", ctx->data);

                result = STATUS_SUCCESS;
                break;

            // New API in v3.7.1. This does not break ABI compat from v3.7.0.
            // If v3.7.0 client calls into v3.7.1 implementation, then the
            // call is a noop - handled by safely returning ENOTSUP.
            // No new structs, no struct layout changes.
            case EVT_OP_SET_LOGGER_CONTEXT:
                result = mat_sendprops(ctx, EVT_OP_SET_LOGGER_CONTEXT);
                break;

            // New API in v3.7.1. This does not break ABI compat from v3.7.0.
            // If v3.7.0 client calls into v3.7.1 implementation, then the
            // call is a noop - handled by safely returning ENOTSUP.
            // No new structs, no struct layout changes.
            case EVT_OP_SET_LOGMANAGER_CONTEXT:
                result = mat_sendprops(ctx, EVT_OP_SET_LOGMANAGER_CONTEXT);
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

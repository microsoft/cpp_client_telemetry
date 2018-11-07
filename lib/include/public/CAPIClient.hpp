#ifndef EVENTSCLIENT_HPP
#define EVENTSCLIENT_HPP
#pragma once

//
// Header-only implementation of C++03 API on top of stable C ABI
//

#include "Version.hpp"

#include "aria.h"

namespace ARIASDK_NS_BEGIN {

    // TODO: [MG] - consider __fastcall for all methods here for best perf
    class CAPIClient
    {

    protected:
        evt_handle_t    handle;
        evt_handle_t    lib;

    public:

        CAPIClient(evt_handle_t lib = 0) :
            handle(0),
            lib(lib)
        {
            if (lib != 0)
                evt_load(lib);
        }

        virtual ~CAPIClient()
        {
            if (lib != 0)
                evt_unload(lib);
        }

        evt_handle_t open(const char* config)
        {
            handle = evt_open(config);
            return handle;
        };

        evt_status_t configure(const char* config)
        {
            return evt_configure(handle, config);
        }

        // TODO: [MG] - header-only EventProperties class?
        evt_status_t log(evt_prop* evt)
        {
            return evt_log(handle, evt);
        }

        evt_status_t pause()
        {
            return evt_pause(handle);
        }

        evt_status_t resume()
        {
            return evt_resume(handle);
        }

        evt_status_t upload()
        {
            return evt_upload(handle);
        }

        evt_status_t flush()
        {
            return evt_flush(handle);
        }

        evt_status_t close()
        {
            return evt_close(handle);
        }

        const char * version()
        {
            // FIXME: [MG] - pass version hardcoded in our header
            return evt_version("1.0.0");
        }

    };

} ARIASDK_NS_END

#endif

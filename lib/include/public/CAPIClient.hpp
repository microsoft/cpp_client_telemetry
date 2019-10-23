// Copyright (c) Microsoft. All rights reserved.
#ifndef EVENTSCLIENT_HPP
#define EVENTSCLIENT_HPP
//
// Header-only implementation of C++03 API on top of stable C ABI
//

#include "Version.hpp"

#include "mat.h"

/* This class is currently incompatible with Secure Template Overloads */
#ifdef evt_log
#undef evt_log
#endif

namespace ARIASDK_NS_BEGIN
{

    class CAPIClient
    {

    protected:
        evt_handle    handle;
        evt_handle    lib;

    public:

        CAPIClient(evt_handle lib = 0) :
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

        evt_handle open(const char* config)
        {
            handle = evt_open(config);
            return handle;
        };

        evt_status configure(const char* config)
        {
            return evt_configure(handle, config);
        }

        // TODO: [MG] - header-only EventProperties class?
        evt_status log(evt_prop* evt)
        {
            return evt_log(handle, evt);
        }

        evt_status pause()
        {
            return evt_pause(handle);
        }

        evt_status resume()
        {
            return evt_resume(handle);
        }

        evt_status upload()
        {
            return evt_upload(handle);
        }

        evt_status flush()
        {
            return evt_flush(handle);
        }

        evt_status close()
        {
            return evt_close(handle);
        }

        const char * version()
        {
            return evt_version();
        }

    };

} ARIASDK_NS_END

#endif

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
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

namespace MAT_NS_BEGIN
{

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
            return evt_version();
        }

    };

} MAT_NS_END

#endif

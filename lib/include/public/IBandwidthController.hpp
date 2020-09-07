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
#ifndef IBANDWIDTHCONTROLLER_HPP
#define IBANDWIDTHCONTROLLER_HPP

#include "Version.hpp"

namespace MAT_NS_BEGIN
{
    class IBandwidthController {
    public:
        virtual ~IBandwidthController() {}

        /// <summary>
        /// Query the current proposed bandwidth for uploading telemetry events.
        ///
        /// This method is called each time the SDK is preparing to send some
        /// events in an HTTP request.
        ///
        /// The library might not be able to actually honor the return value more
        /// than just not transmitting anything if the proposed bandwidth is 0 and
        /// resuming the upload when it isn't.
        /// </summary>
        /// <returns>Proposed bandwidth in bytes per second</returns>
        virtual unsigned GetProposedBandwidthBps() = 0;
    };
} MAT_NS_END

#endif

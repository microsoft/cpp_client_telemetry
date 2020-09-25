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
#ifndef IDATAVIEWER_HPP
#define IDATAVIEWER_HPP

#include "Version.hpp"
#include "ctmacros.hpp"
#include "IModule.hpp"

#include <string>
#include <vector>

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// This interface allows SDK users to register a data viewer
    /// that will receive all packets uploaded by the SDK.
    /// </summary>
    class IDataViewer: public IModule
    {
    public:
        /// <summary>
        /// This method allows SDK to pass the uploaded packet to the data viewer.
        /// </summary>
        /// <param name="packetData">HTTP Request Packet as a binary blob.</param>
        virtual void ReceiveData(const std::vector<uint8_t>& packetData) noexcept = 0;

        /// <summary>
        /// Get the name of the current viewer.
        /// </summary>
        virtual const char* GetName() const noexcept = 0;

        /// <summary>
        /// Check if the current viewer is transmitting.
        /// </summary>
        /// <returns>True if transmission is enabled, false otherwise.</returns>
        virtual bool IsTransmissionEnabled() const noexcept = 0;

        /// <summary>
        /// Get the current endpoint where the data is being streamed to.
        /// </summary>
        /// <returns>const char* denoting the endpoint, empty string if not currently streaming.</returns>
        virtual const std::string& GetCurrentEndpoint() const noexcept = 0;
    };

} MAT_NS_END

#endif

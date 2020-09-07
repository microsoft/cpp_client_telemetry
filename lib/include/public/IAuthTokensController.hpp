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
#ifndef MAT_IAUTHTOKENS_HPP
#define MAT_IAUTHTOKENS_HPP

#include "Version.hpp"
#include "ctmacros.hpp"
#include "Enums.hpp"

namespace MAT_NS_BEGIN
{
    const char* const TICKETS_PREPEND_STRING = "1000";

    /// <summary>
    /// This class is used to manage the Events  logging system
    /// </summary>
    class MATSDK_LIBABI IAuthTokensController
    {
    public:
        /// <summary>
        /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
        /// </summary>
        virtual ~IAuthTokensController() {}

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual status_t  SetTicketToken(TicketType type, char const* tokenValue) = 0;

        /// <summary>
        /// Clears all tokens.
        /// </summary>
        virtual status_t  Clear() = 0;

        /// <summary>
        /// sets strict mode for application( all tokens in that app).
        /// </summary>
        virtual status_t  SetStrictMode(bool value) = 0;

        /// <summary>
        /// gets strict mode for application.
        /// </summary>
        virtual bool  GetStrictMode() = 0;

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::vector<std::string>&  GetTickets() = 0;

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>&  GetDeviceTokens() = 0;

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>&  GetUserTokens() = 0;

    };

} MAT_NS_END

#endif //MAT_IAUTHTOKENS_HPP
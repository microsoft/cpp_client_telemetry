//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_IAUTHTOKENS_HPP
#define MAT_IAUTHTOKENS_HPP

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

// Copyright (c) Microsoft. All rights reserved.
#ifndef ARIA_IAUTHTOKENS_HPP
#define ARIA_IAUTHTOKENS_HPP

#include "ctmacros.hpp"
#include "Enums.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
// *INDENT-ON*
/// <summary>
/// This class is used to manage the Events  logging system
/// </summary>
    class ARIASDK_LIBABI IAuthTokensController
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
        virtual EVTStatus  SetTicketToken(TicketType type, char const* tokenValue) = 0;

        /// <summary>
        /// clears all tokens.
        /// </summary>
        virtual EVTStatus  Clear() = 0;

    };


}}} // namespace Microsoft::Applications::Events 

#endif //ARIA_IAUTHTOKENS_HPP
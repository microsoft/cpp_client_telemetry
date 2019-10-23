// Copyright (c) Microsoft. All rights reserved.
#ifndef AUTHTOKENSCONTROLLER_HPP
#define AUTHTOKENSCONTROLLER_HPP

#include "Enums.hpp"
#include "IAuthTokensController.hpp"
#include "Version.hpp"
#include "pal/PAL.hpp"

#include <map>
#include <vector>

// *INDENT-OFF*
namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// This class is used to manage the Auth Tokens
    /// </summary>
    class AuthTokensController : public IAuthTokensController
    {
       public:
        AuthTokensController();
        /// <summary>
        /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
        /// </summary>
        virtual ~AuthTokensController();

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="tokenValue">Auth ticket (token) value</param>
        virtual status_t SetTicketToken(TicketType type, char const* tokenValue) override;

        /// <summary>
        /// Clears all tokens.
        /// </summary>
        virtual status_t Clear() override;

        /// <summary>
        /// sets strict mode for application( all tokens in that app).
        /// </summary>
        virtual status_t SetStrictMode(bool value) override;

        /// <summary>
        /// gets strict mode for application.
        /// </summary>
        virtual bool GetStrictMode() override;

        /// <summary>
        /// Get the Auth tickets collection.
        /// </summary>
        virtual std::vector<std::string>& GetTickets() override;

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>& GetDeviceTokens() override;

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>& GetUserTokens() override;

       private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();
        std::map<TicketType, std::string> m_deviceTokens;
        std::map<TicketType, std::string> m_userTokens;
        std::vector<std::string> m_tickets;
        bool m_IsStrictModeEnabled;
    };

}
ARIASDK_NS_END

#endif

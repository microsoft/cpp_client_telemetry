// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "Enums.hpp"
#include "pal/PAL.hpp"
#include "IAuthTokensController.hpp"
#include <map>
#include <vector>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


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
    /// <param name="ticketValue">Ticketvalue</param>
    virtual ACTStatus  SetTicketToken(TicketType type, char const* tokenValue);

    /// <summary>
    /// Set the Auth ticket.
    /// </summary>
    /// <param name="type">Ticket type</param>
    /// <param name="ticketValue">Ticketvalue</param>
    virtual std::vector<std::string>&  GetTickets();
    
    /// <summary>
    /// Set the Auth ticket.
    /// </summary>
    /// <param name="type">Ticket type</param>
    /// <param name="ticketValue">Ticketvalue</param>
    virtual std::map<TicketType, std::string>&  GetDeviceTokens();

    /// <summary>
    /// Set the Auth ticket.
    /// </summary>
    /// <param name="type">Ticket type</param>
    /// <param name="ticketValue">Ticketvalue</param>
    virtual std::map<TicketType, std::string>&  GetUserTokens();

  private:
      ARIASDK_LOG_DECL_COMPONENT_CLASS();
      std::map<TicketType, std::string> m_deviceTokens;
      std::map<TicketType, std::string> m_userTokens;
      std::vector<std::string> m_tickets;
};


}}} // namespace Microsoft::Applications::Telemetry

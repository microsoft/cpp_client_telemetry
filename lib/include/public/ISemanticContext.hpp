
#ifndef ISEMANTICCONTEXT_HPP
#define ISEMANTICCONTEXT_HPP

#include "ctmacros.hpp"
#include "Enums.hpp"
#include <string>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*

class  ARIASDK_LIBABI ISemanticContext
{
  public:
    virtual  ~ISemanticContext() {}

    /// <summary>
    /// Set the application identifier context information of telemetry event.
    /// </summary>
    /// <param name="appId">Id that uniquely identifies the user-facing application from which this event originated</param>
    virtual void  SetAppId(std::string const& appId) = 0;

    /// <summary>
    /// Set the application version context information of telemetry event.
    /// </summary>
    /// <param name="appVersion">Version of the application, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetAppVersion(std::string const& appVersion) = 0;

    /// <summary>
    /// Set the application language context information of telemetry event.
    /// </summary>
    virtual void  SetAppLanguage(std::string const& appLanguage) = 0;

	/// <summary>
	/// Set the application's experiment IDs information of telemetry event.
	/// The experiment IDs information will be applied to all events unless it is overwritten by that set via SetEventExperimentIds  
	/// </summary>
	/// <param name="appVersion">list of IDs of experimentations into which the application is enlisted</param>
    virtual void  SetAppExperimentIds(std::string const& appExperimentIds) = 0;

	/// <summary>
	/// Set the application version context information of telemetry event.
	/// Removes previously stored experiment ids set by SetAppExperimentIds.
	/// </summary>
	/// <param name="appVersion">ETAG which is a hash of the set of experimentations into which the application is enlisted</param>
	virtual void  SetAppExperimentETag(std::string const& appExperimentETag) = 0;

	/// <summary>
	/// Set the application experimentation impression id information of telemetry event.
	/// </summary>
	/// <param name="appExperimentIds">List of expementation IDs which are app/platform specific</param>
	virtual void  SetAppExperimentImpressionId(std::string const& appExperimentImpressionId) = 0;
	
	/// <summary>
	/// Set the experiment IDs information of the specified telemetry event.
	/// </summary>
	/// <param name="appVersion">list of IDs of experimentations into which the application is enlisted</param>
	virtual void  SetEventExperimentIds(std::string const& eventName, std::string const& experimentIds) = 0;
/*
    /// <summary>
    /// Set the device identifier context information of telemetry event.
    /// </summary>
    /// <param name="deviceId">A unique device identifier, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetDeviceId(std::string const& deviceId) = 0;

    /// <summary>
    /// Set the device manufacturer context information of telemetry event.
    /// </summary>
    /// <param name="deviceMake">The manufacturer of the device, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetDeviceMake(std::string const& deviceMake) = 0;

    /// <summary>
    /// Set the device model context information of telemetry event.
    /// </summary>
    /// <param name="deviceModel">The model of the device, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetDeviceModel(std::string const& deviceModel) = 0;

    /// <summary>
    /// Set the network cost context information of telemetry event.
    /// </summary>
    /// <param name="networkCost">The cost of using data traffic on the current network</param>
    virtual void  SetNetworkCost(NetworkCost networkCost) = 0;

    /// <summary>
    /// Set the network provider context information of telemetry event.
    /// </summary>
    /// <param name="networkProvider">The provider used to connect to the current network, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetNetworkProvider(std::string const& networkProvider) = 0;

    /// <summary>
    /// Set the network type context information of telemetry event.
    /// </summary>
    /// <param name="networkType">The type of the current network</param>
    virtual void  SetNetworkType(NetworkType networkType) = 0;

    /// <summary>
    /// Set the system name context information of telemetry event.
    /// </summary>
    /// <param name="osName">The system anme, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetOsName(std::string const& osName) = 0;

    /// <summary>
    /// Set the system version context information of telemetry event.
    /// </summary>
    /// <param name="osVersion">The system version, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetOsVersion(std::string const& osVersion) = 0;

    /// <summary>
    /// Set the system build number context information of telemetry event.
    /// </summary>
    /// <param name="osBuild">The system build, retrieved programmatically where possible and is app/platform specific</param>
    virtual void  SetOsBuild(std::string const& osBuild) = 0;
*/
    /// <summary>
    /// Set the userId context information of telemetry event.
    /// </summary>
    /// <param name="userId">Identifier that uniquely identifies a user in the application-specific user namespace</param>
    /// <param name='piiKind'>PIIKind of the userId. Default to PiiKind_Identity, set it to PiiKind_None to denote it as non-PII.</param>
    virtual void  SetUserId(std::string const& userId, PiiKind piiKind = PiiKind_Identity) = 0;
/*
    /// <summary>
    /// Set the user MsaId context information of telemetry event.
    /// </summary>
    /// <param name="userMsaId">Msa id that identifies a user in the application-specific user namespace</param>
    virtual void  SetUserMsaId(std::string const& userMsaId) = 0;

    /// <summary>
    /// Set the user ANID context information of telemetry event.
    /// </summary>
    /// <param name="userANID">ANID that identifies a user in in the application-specific user namespace</param>
    virtual void  SetUserANID(std::string const& userANID) = 0;

    /// <summary>
    /// Set the advertising Id context information of telemetry event.
    /// </summary>
    /// <param name="userAdvertingId">Advertising Id of a user to use in an application-specific user namespace</param>
    virtual void  SetUserAdvertisingId(std::string const& userAdvertingId) = 0;
*/
    /// <summary>
    /// Set the user language context information of telemetry event.
    /// </summary>
    /// <param name="locale">user's language in IETF language tag format, as described in RFC 4646.</param>
    virtual void  SetUserLanguage(std::string const& locale) = 0;

    /// <summary>
    /// Set the user time zone context information of telemetry event.
    /// </summary>
    /// <param name="timeZone">user's time zone relative to UTC, in ISO 8601 time zone format</param>
    virtual void  SetUserTimeZone(std::string const& timeZone) = 0;

    /// <summary>
    /// Set the Auth ticket.
    /// </summary>
    /// <param name="type">Ticket type</param>
    /// <param name="ticketValue">Ticketvalue</param>
    virtual void  SetTicket(TicketType type, std::string const& ticketValue) = 0;
    
};


}}} // namespace Microsoft::Applications::Telemetry

#endif //ISEMANTICCONTEXT_H